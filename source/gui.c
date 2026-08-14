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
#include "presets.h"
#include "widgets.h"

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


#define DEFAULT_PANEL_WIDTH     325
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

#define COLOR_RANGE 255.0f

#define SLIDER_PADDING  20
#define SLIDER_WIDTH    ( DEFAULT_PANEL_WIDTH - (3 * SLIDER_PADDING ) )
#define SLIDER_HEIGHT   DEFAULT_WIDGET_HEIGHT

// Which particle color is currently being edited
static int active_color_index = -1;

/**
 * update_world_section
 *
 * @brief Builds the "Particle Settings" tree: count, class count, and colors.
 *
 * Emits the particle-count slider (raising dirty_count when changed), the class
 * count slider, a row of per-class color swatches that open a color-picker popup,
 * and a color-preset combobox plus a randomize button. Edits write directly into
 * the tunable palette and counts.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal helper and should only be called from update_gui().
 *       The currently edited swatch is tracked in the file-static active_color_index.
 */
static void update_world_section(application_t *application) {
    struct nk_context *ctx = application->gui_context;
    uint32_t * const nclasses = &application->tunables.nclass;
    uint32_t * const particle_count = &application->tunables.particle_count;
    uint32_t * const new_count = &application->tunables.new_count;

    // Particle Settings Section
    if (!nk_tree_push(ctx, NK_TREE_TAB, "PARTICLE SETTINGS", NK_MAXIMIZED))
        return;
    
    // Particle Count Label
    nk_layout_row_static(ctx, 20, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(ctx, "Particle Count", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    // Particle Count Slider
    nk_labelf(ctx, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM, "%d", *particle_count);
    nk_layout_space_begin(ctx, NK_STATIC, DEFAULT_WIDGET_HEIGHT, 1);
    nk_layout_space_push(ctx, nk_rect(SLIDER_PADDING, 0, SLIDER_WIDTH, SLIDER_HEIGHT));
    if (nk_slider_int(ctx, 0, (int *) new_count, MAX_PARTICLES, 1)) 
        application->tunables.dirty_count = true;
    nk_layout_space_end(ctx);

    // Particle Types Label
    nk_layout_row_static(ctx, 20, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(ctx, "Particle Types", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    // Particle Types Slider
    nk_labelf(ctx, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM, "%d", *nclasses);
    nk_layout_space_begin(ctx, NK_STATIC, DEFAULT_WIDGET_HEIGHT, 1);
    nk_layout_space_push(ctx, nk_rect(SLIDER_PADDING, 0, SLIDER_WIDTH, SLIDER_HEIGHT));
    nk_slider_int(ctx, 1, (int *) nclasses, 8, 1);
    nk_layout_space_end(ctx);

    // Particle Colors Label
    nk_layout_row_static(ctx, 15, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(ctx, "Particle Colors", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);
    // For each color type, display a circle with each corresponding color
    nk_layout_row_dynamic(ctx, DEFAULT_WIDGET_HEIGHT, *nclasses);
    for (size_t i = 0; i < *nclasses; i++) {
        if (nk_button_color(ctx, (struct nk_color) {
            .r = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[i][0]),
            .g = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[i][1]),
            .b = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[i][2]),
            .a = (nk_byte) (COLOR_RANGE)
        } )) {
            active_color_index = (int) i;
        }
    }

    if (active_color_index >= 0) {
        if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Edit Color",
                            NK_WINDOW_CLOSABLE, nk_rect(DEFAULT_PANEL_WIDTH + 10, 0, DEFAULT_PANEL_WIDTH, DEFAULT_PANEL_WIDTH))) {
            nk_layout_row_dynamic(ctx, 220, 1);
            nk_color_pick(ctx, (struct nk_colorf *) application->tunables.rgba_palette[active_color_index], NK_RGBA);
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
                memcpy(application->tunables.rgba_palette, color_presets[i], sizeof(application->tunables.rgba_palette));
            }
        }
        nk_combo_end(ctx);
    }
    if (nk_button_label(ctx, "Randomize")) {
        for (size_t i = 0; i < *nclasses; i++) {
            application->tunables.rgba_palette[i][0] = SDL_randf();
            application->tunables.rgba_palette[i][1] = SDL_randf();
            application->tunables.rgba_palette[i][2] = SDL_randf();
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
 * where each cell's color encodes its weight (red = repel, blue = attract). Clicking
 * a cell selects it; a property widget below then edits the selected weight, and a
 * preset combobox and randomize button rewrite the whole matrix. Any change raises
 * dirty_matrix so the GPU buffer is refreshed.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal helper and should only be called from update_gui().
 *       The selected cell/row/col persist across frames in function-local statics.
 */
static void update_attraction_matrix_section(application_t *application) {
    struct nk_context *ctx   = application->gui_context;
    uint8_t const nclasses   = application->tunables.nclass;
    uint8_t const dimensions = nclasses + 1; // +1 for column and row headers
    float *matrix = application->attraction.matrix;
    uint32_t matrix_length = application->attraction.length;
    
    // Attraction Matrix Section
    if (!nk_tree_push(ctx, NK_TREE_TAB, "ATTRACTION MATRIX", NK_MINIMIZED))
        return;

    // Attraction Matrix Grid Positioning Constants
    float const step = MATRIX_CELL_SIZE + MATRIX_CELL_PADDING;
    float const span = dimensions * MATRIX_CELL_SIZE + (dimensions - 1) * MATRIX_CELL_PADDING;
    float const available = nk_window_get_content_region_size(ctx).x;
    float       x0 = (available - span) * 0.5f;
    if (x0 < 0.0f) x0 = 0.0f;   // if panel is too narrow, pin content left
    
    nk_layout_space_begin(ctx, NK_STATIC, span, dimensions * dimensions);
    static uint8_t active_cell = 0;
    static uint8_t active_row  = 1;
    static uint8_t active_col  = 1;
    static bool universal      = true;

    for (uint8_t r = 0; r < dimensions; r++) {
        for (uint8_t c = 0; c < dimensions; c++) {
            nk_layout_space_push(ctx, nk_rect(x0 + c * step, r * step, MATRIX_CELL_SIZE, MATRIX_CELL_SIZE));
            
            if (r == 0 && c == 0) { // Representation for all particles
                struct nk_style_button all_colors = rainbow_button(ctx);
                if (nk_button_label_styled(ctx, &all_colors, EMPTY_STRING)) {
                    universal = true;
                }
            }
            else if (r > 0  && c > 0) { // attraction factor cell
                uint8_t matrix_idx = (r - 1) * MAX_NUM_CLASSES + (c - 1);
                struct nk_style_button factor = attraction_factor_button(ctx, &matrix[matrix_idx]);
                if (nk_button_label_styled(ctx, &factor, EMPTY_STRING)) {
                    active_cell = matrix_idx;
                    active_row = r;
                    active_col = c;
                    universal = false;
                }
            }
            else { // header cell
                uint8_t palette_idx = (r == 0) ? (c - 1) : (r - 1); // determine the header for row and col
                struct nk_style_button header = circular_button(ctx, (struct nk_color) {
                    .r = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[palette_idx][0]),
                    .g = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[palette_idx][1]),
                    .b = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[palette_idx][2]),
                    .a = (nk_byte) (COLOR_RANGE)
                } );
                nk_button_label_styled(ctx, &header, EMPTY_STRING);
            }
        }
    }
    nk_layout_space_end(ctx);

    // Now for tuning of the cell value
    struct nk_style_button active;
    struct nk_style_button affected;

    uint8_t palette_idx0 = active_row - 1;
    uint8_t palette_idx1 = active_col - 1;

    if (universal) {
        active   = rainbow_button(ctx);
        affected = rainbow_button(ctx);
    }
    else {
        active = circular_button(ctx, (struct nk_color) {
            .r = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[palette_idx0][0]),
            .g = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[palette_idx0][1]),
            .b = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[palette_idx0][2]),
            .a = (nk_byte) (COLOR_RANGE)
        } );
        affected = circular_button(ctx, (struct nk_color) {
            .r = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[palette_idx1][0]),
            .g = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[palette_idx1][1]),
            .b = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[palette_idx1][2]),
            .a = (nk_byte) (COLOR_RANGE)
        } );
    }

    nk_layout_row(ctx, NK_DYNAMIC, MATRIX_CELL_SIZE, 4, (float const []) {0.1f, 0.1f, 0.1f, 0.7f});
    nk_button_label_styled(ctx, &active, EMPTY_STRING);
    
    struct nk_rect bounds;
    nk_widget(&bounds, ctx);
    nk_draw_symbol(nk_window_get_canvas(ctx),
                   NK_SYMBOL_TRIANGLE_RIGHT,
                   nk_shrink_rect(bounds, 8.0f),
                   nk_rgba(0, 0, 0, 0),
                   ctx->style.text.color,
                   1.0f,
                   ctx->style.font);
    nk_button_label_styled(ctx, &affected, EMPTY_STRING);

    static float new_attraction_value = 0.0f;
    if (!universal) 
        new_attraction_value = matrix[active_cell];
    if (nk_property_float(ctx, "attraction", -1.0f, &new_attraction_value, 1.0f, 0.1f, 0.01f)) {
        if (universal) {
            for (size_t i = 0; i < matrix_length; ++i) 
                matrix[i] = new_attraction_value;
        }
        else matrix[active_cell] = new_attraction_value;

        application->tunables.dirty_matrix = true;
    }

    // attraction presets
    // combobox and randomize button
    nk_layout_row(ctx, NK_DYNAMIC, 25, 2, (float const []) { 0.7f, 0.3f });
    if (nk_combo_begin_label(ctx, "Attraction Presets",
                              nk_vec2(DEFAULT_PANEL_WIDTH - PANEL_CONTENT_RIGHT_PADDING, 200))) {
        nk_layout_row_dynamic(ctx, 25, 1);
        for (uint8_t i = 0; i < MATRIX_PRESET_COUNT; i++) {
            if (nk_combo_item_label(ctx, matrix_preset_names[i], NK_TEXT_LEFT)) {
                memcpy(matrix, matrix_presets[i], sizeof(float) * matrix_length);
                application->tunables.dirty_matrix = true;
            }
        }
        nk_combo_end(ctx);
    }
    if (nk_button_label(ctx, "Randomize")) {
        for (size_t i = 0; i < matrix_length; i++) {
            matrix[i] = SDL_randf() * 2.0f - 1.0f;
        }
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
    nk_layout_row_static(ctx, 20, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(ctx, "Friction", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    // Friction Half-Life Slider
    nk_labelf(ctx, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM, "%g", *friction);
    nk_layout_space_begin(ctx, NK_STATIC, DEFAULT_WIDGET_HEIGHT, 1);
    nk_layout_space_push(ctx, nk_rect(SLIDER_PADDING, 0, SLIDER_WIDTH, SLIDER_HEIGHT));
    nk_slider_float(ctx, 0.0f, friction, 3.0f, 0.25f);
    nk_layout_space_end(ctx);
    
    // Delta Time Label
    nk_layout_row_static(ctx, 20, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(ctx, "Delta Time", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    // Delta Time Slider
    nk_labelf(ctx, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM, "%g", *deltatime);
    nk_layout_space_begin(ctx, NK_STATIC, DEFAULT_WIDGET_HEIGHT, 1);
    nk_layout_space_push(ctx, nk_rect(SLIDER_PADDING, 0, SLIDER_WIDTH, SLIDER_HEIGHT));
    nk_slider_float(ctx, 0.025f, deltatime, 1.0f, 0.025f);
    nk_layout_space_end(ctx);

    // Attraction Radius Label
    nk_layout_row_static(ctx, 20, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(ctx, "Attraction Radius", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    // Attraction Radius Slider
    nk_labelf(ctx, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM, "%g", *aradius);
    nk_layout_space_begin(ctx, NK_STATIC, DEFAULT_WIDGET_HEIGHT, 1);
    nk_layout_space_push(ctx, nk_rect(SLIDER_PADDING, 0, SLIDER_WIDTH, SLIDER_HEIGHT));
    nk_slider_float(ctx, 0.0f, aradius, 200.0f, 5.0f);
    nk_layout_space_end(ctx);

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