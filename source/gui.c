/**
 * @file gui.c
 * @brief Nuklear immediate-mode GUI integration for the SDL3 / OpenGL backend.
 *
 * This is the single translation unit that defines NK_IMPLEMENTATION and
 * NK_SDL_GL3_IMPLEMENTATION, so the Nuklear library and its SDL3/GL3 backend
 * bodies are emitted here exactly once. Every other file gets declaration-only
 * Nuklear through gui.h.
 */
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "application.h"
#include "gui.h"
#include "widgets.h"
#include "particle.h"

/**
 * init_gui
 *
 * @brief Initializes the Nuklear GUI context and its default font.
 *
 * Creates the Nuklear rendering device on the application's SDL window, then
 * bakes and uploads the built-in default font atlas so the context has a usable
 * font. The application window and OpenGL context must already be initialized
 * before calling this.
 *
 * @param application  Pointer to the initialized application state; gui_context
 *                     is set on success.
 * @return             1 on success, 0 if the GUI context could not be created.
 *
 * @note The font stash step is required — without a baked font, the first call
 *       into nk_begin() asserts that a font is missing.
 * @see  destroy_gui(), update_gui()
 */
bool init_gui(application_t *application) {
    application->gui_context = nk_sdl_init(application->window);
    if (application->gui_context == NULL)
        return false;

    // Load default font
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    return true;
}

/**
 * destroy_gui
 *
 * @brief Tears down the Nuklear GUI context and its GPU resources.
 *
 * Frees the Nuklear context, font atlas, and the backend's OpenGL objects
 * (shader, buffers, font texture). Must be called while the OpenGL context is
 * still current — i.e. before the SDL GL context is destroyed.
 *
 * @return  1 on success.
 *
 * @see init_gui()
 */
bool destroy_gui(void) {
    nk_sdl_shutdown();

    return true;
}


#define PANEL_CONTENT_RIGHT_PADDING 20
#define DEFAULT_WIDGET_HEIGHT   35
#define MAX_NUM_COUNT           8
#define EMPTY_STRING            ""

/**
 * update_state_section
 *
 * @brief Builds the simulation-state controls (play/pause and shuffle).
 *
 * Emits a Pause button while RUNNING or a Play button while PAUSED (toggling
 * application->state), followed by a Shuffle button that requests a randomized
 * restart by raising the shuffle flag.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal helper and should only be called from update_gui().
 */
static void update_state_section(application_t *application) {
    struct nk_context *ctx = application->gui_context;

    // Simulation State Section
    nk_layout_row(ctx, NK_DYNAMIC, DEFAULT_WIDGET_HEIGHT, 3, (float const []) { 0.05f, 0.45f, 0.45f });
    nk_spacer(ctx);

    if (application->state == RUNNING) {
        if (nk_button_symbol_label(ctx, NK_SYMBOL_RECT_SOLID, "  PAUSE", NK_TEXT_ALIGN_RIGHT)) {
            application->state = PAUSED;
        }
    }
    else if (application->state == PAUSED) {
        if (nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_RIGHT, "  PLAY", NK_TEXT_ALIGN_RIGHT)) {
            application->state = RUNNING;
        }
    }

    if (nk_button_label(ctx, "Shuffle")) application->tunables.shuffle = true;
}

#define SLIDER_PADDING  10
#define SLIDER_WIDTH    ( DEFAULT_PANEL_WIDTH - (3 * SLIDER_PADDING ) )
#define SLIDER_HEIGHT   DEFAULT_WIDGET_HEIGHT


// Which particle color is currently being edited
static int active_color_index = -1;

/**
 * update_world_section
 *
 * @brief Builds the "Particle Settings" tree: count, class count, and colors.
 *
 * Emits the particle-count and class-count slider+textbox combos (see
 * uint_variable_slider()), raising dirty_count when either changes, a row of
 * per-class color swatches that open an extended color-picker popup, and a
 * color-preset combobox plus a randomize button. Edits write directly into the
 * tunable palette and counts.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal helper and should only be called from update_gui().
 *       The currently edited swatch is tracked in the file-static active_color_index.
 */
static void update_world_section(application_t *application) {
    struct nk_context *ctx = application->gui_context;
    uint32_t * const nclasses = &application->tunables.nclass;
    uint32_t * const new_count = &application->tunables.new_count;
    float (*palette)[NUM_CHANNELS] = application->tunables.rgba_palette;

    // Particle Settings Section
    if (!nk_tree_push(ctx, NK_TREE_TAB, "PARTICLE SETTINGS", NK_MAXIMIZED))
        return;
    
    // Particle Count Label
    static char count_slider_text[TEXT_MAX_SIZE] = { '\0' };
    if (uint_variable_slider(ctx, "Count", 0, new_count, MAX_PARTICLES, count_slider_text))
        application->tunables.dirty_count = true;

    static char types_slider_text[TEXT_MAX_SIZE] = { '\0' };
    if (uint_variable_slider(ctx, "Types", 1, nclasses, MAX_NUM_CLASSES, types_slider_text))
        application->tunables.dirty_count = true;

    // Particle Colors Label
    nk_layout_row_static(ctx, 15, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(ctx, "Colors", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);
    // For each color type, display a circle with each corresponding color
    nk_layout_row_dynamic(ctx, DEFAULT_WIDGET_HEIGHT, *nclasses);
    for (size_t i = 0; i < *nclasses; i++) {
        if (nk_button_color(ctx, (struct nk_color) {
            .r = (nk_byte) (COLOR_RANGE * palette[i][0]),
            .g = (nk_byte) (COLOR_RANGE * palette[i][1]),
            .b = (nk_byte) (COLOR_RANGE * palette[i][2]),
            .a = (nk_byte) (COLOR_RANGE)
        } )) {
            active_color_index = (int) i;
        }
    }

    if (active_color_index >= 0) {
        if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Edit Color",
                            NK_WINDOW_CLOSABLE, nk_rect(DEFAULT_PANEL_WIDTH, 0, DEFAULT_PANEL_WIDTH, DEFAULT_PANEL_WIDTH + DEFAULT_WIDGET_HEIGHT * 2))) {
            extended_color_picker(ctx, palette[active_color_index]);
            nk_popup_end(ctx);
        } 
        else active_color_index = -1;
    }

    // combobox and randomize button
    nk_layout_row(ctx, NK_DYNAMIC, 25, 2, (float const []) { 0.7f, 0.3f });
    if (nk_combo_begin_label(ctx, "Color Presets",
                              nk_vec2(DEFAULT_PANEL_WIDTH - PANEL_CONTENT_RIGHT_PADDING, 200))) {
        nk_layout_row_dynamic(ctx, 25, 1);
        for (uint8_t i = 0; i < COLOR_PRESET_COUNT; i++) {
            if (nk_combo_item_label(ctx, color_preset_names[i], NK_TEXT_LEFT)) {
                memcpy(palette, color_presets[i], (MAX_NUM_CLASSES * NUM_CHANNELS) * sizeof(float));
            }
        }
        nk_combo_end(ctx);
    }
    if (nk_button_label(ctx, "Randomize")) {
        for (size_t i = 0; i < *nclasses; i++) {
            palette[i][0] = SDL_randf();
            palette[i][1] = SDL_randf();
            palette[i][2] = SDL_randf();
        }
    }

    nk_tree_pop(ctx);
}

#define MATRIX_CELL_SIZE      28.0f
#define MATRIX_CELL_PADDING   4.0f

/**
 * update_attraction_matrix_section
 *
 * @brief Builds the "Attraction Matrix" tree: the interactive nclass x nclass grid.
 *
 * Lays out a centered grid of colored class headers and attraction-factor cells,
 * where each cell's color encodes its weight (red = repel, blue = attract).
 * Clicking a cell selects just that cell; Ctrl-clicking adds cells to the
 * selection so several can be edited together; clicking the corner button
 * selects the whole matrix. A slider+textbox combo below (see cell_editor())
 * then edits the weight for every cell the current selection covers, and a
 * preset combobox and randomize button rewrite the whole matrix regardless of
 * selection. Any change raises dirty_matrix so the GPU buffer is refreshed.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal helper and should only be called from update_gui().
 *       The selection persists across frames in a function-local static.
 */
static void update_attraction_matrix_section(application_t *application) {
    struct nk_context *ctx   = application->gui_context;
    uint8_t const nclasses   = application->tunables.nclass;
    uint8_t const dimensions = nclasses + 1; // +1 for column and row headers
    attraction_t *attraction = &application->attraction;
    float (*palette)[NUM_CHANNELS] = application->tunables.rgba_palette;
    
    // Attraction Matrix Section
    if (!nk_tree_push(ctx, NK_TREE_TAB, "ATTRACTION MATRIX", NK_MINIMIZED))
        return;

    static active_cell_t cell_data = {
        .selection = { false },
        .rowidx    = 1,
        .colidx    = 1,
        .universal = true
    };

    create_grid(ctx, attraction->matrix, palette, dimensions, &cell_data);

    // Now for tuning of the cell value
    struct nk_style_button active;
    struct nk_style_button affected;

    if (cell_data.universal) {
        active   = rainbow_button(ctx, true);
        affected = rainbow_button(ctx, true);
    }
    else {
        active = circular_button(ctx, (struct nk_color) {
            .r = (nk_byte) (COLOR_RANGE * palette[cell_data.rowidx - 1][0]),
            .g = (nk_byte) (COLOR_RANGE * palette[cell_data.rowidx - 1][1]),
            .b = (nk_byte) (COLOR_RANGE * palette[cell_data.rowidx - 1][2]),
            .a = (nk_byte) (COLOR_RANGE) } );
        affected = circular_button(ctx, (struct nk_color) {
            .r = (nk_byte) (COLOR_RANGE * palette[cell_data.colidx - 1][0]),
            .g = (nk_byte) (COLOR_RANGE * palette[cell_data.colidx - 1][1]),
            .b = (nk_byte) (COLOR_RANGE * palette[cell_data.colidx - 1][2]),
            .a = (nk_byte) (COLOR_RANGE) } );
    }

    if (cell_editor(ctx, attraction, cell_data, active, affected)) {
        application->tunables.dirty_matrix = true;
    }
    if (grid_preset_randomize(ctx, attraction)) {
        application->tunables.dirty_matrix = true;
    }

    nk_tree_pop(ctx);
}

/**
 * update_physics_section
 *
 * @brief Builds the "Physics" tree: friction, delta time, and attraction radius sliders.
 *
 * Emits three sliders that write directly into the physics tunables. These feed the
 * compute shader as uniforms each frame, so changes take effect immediately without
 * a dirty flag.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal helper and should only be called from update_gui().
 */
static void update_physics_section(application_t *application) {
    struct nk_context *ctx = application->gui_context;
    float *friction  = &application->tunables.friction_halflife;
    float *deltatime = &application->tunables.delta_time;
    float *aradius   = &application->tunables.attraction_radius;

    // Physics Section
    if (!nk_tree_push(ctx, NK_TREE_TAB, "PHYSICS", NK_MAXIMIZED))
        return;

    // Friction Half Life
    static char friction_slider_text[TEXT_MAX_SIZE] = { '\0' };
    float_variable_slider(ctx, "Friction", 0.0f, friction, 3.0f, 0.25f, friction_slider_text);
    
    // Delta Time
    static char dt_slider_text[TEXT_MAX_SIZE] = { '\0' };
    float_variable_slider(ctx, "Delta Time", 0.025f, deltatime, 1.0f, 0.025f, dt_slider_text);

    // Attraction Radius
    static char ar_slider_text[TEXT_MAX_SIZE] = { '\0' };
    float_variable_slider(ctx, "Max Radius", 0.0f, aradius, 200.0f, 5.0f, ar_slider_text);

    nk_tree_pop(ctx);
}

/**
 * update_gui
 *
 * @brief Builds the GUI layout for the current frame.
 *
 * Records the widgets for this frame into Nuklear's command buffer. This only
 * describes the UI; the recorded commands are converted to draw calls later by
 * nk_sdl_render(). Must be called once per frame, after input has been fed to
 * the context and before the frame is rendered.
 *
 * @param application  Pointer to the running application state, used for the
 *                     GUI context.
 *
 * @note Widget interactions (e.g. button presses) are handled inline here.
 * @see  init_gui(), mainloop()
 */
void update_gui(application_t *application) {
    if (nk_begin(application->gui_context, "Control Panel", nk_rect(0, 0, DEFAULT_PANEL_WIDTH, application->height), 
        NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE)
    ) {
        update_state_section(application);
        update_world_section(application);
        update_attraction_matrix_section(application);
        update_physics_section(application);
    }
    nk_end(application->gui_context);
}