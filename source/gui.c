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

#include "SDL.h"

#include "application.h"
#include "gui.h"
#include "presets.h"

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


#define DEFAULT_PANEL_WIDTH     300
#define PANEL_CONTENT_RIGHT_PADDING 20
#define DEFAULT_WIDGET_HEIGHT   35
#define MAX_NUM_COUNT           8

static void update_state_section(application_t *application) {
    // Simulation State Section
    nk_layout_row_dynamic(application->gui_context, DEFAULT_WIDGET_HEIGHT, 1);
    nk_label(application->gui_context, "SIMULATION STATE", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

    static const float button_ratio[] = { 0.05f, 0.45f, 0.45f };
    nk_layout_row(application->gui_context, NK_DYNAMIC, DEFAULT_WIDGET_HEIGHT, 3, button_ratio);
    nk_spacer(application->gui_context);

    if (application->state == RUNNING) {
        if (nk_button_symbol_label(application->gui_context, NK_SYMBOL_RECT_SOLID, "  PAUSE", NK_TEXT_ALIGN_RIGHT)) {
            application->state = PAUSED;
        }
    }
    else if (application->state == PAUSED) {
        if (nk_button_symbol_label(application->gui_context, NK_SYMBOL_TRIANGLE_RIGHT, "  PLAY", NK_TEXT_ALIGN_RIGHT)) {
            application->state = RUNNING;
        }
    }

    if (nk_button_label(application->gui_context, "Shuffle")) {
        application->tunables.shuffle = true;
    }
}

#define COLOR_RANGE 255.0f

// Which particle color is currently being edited
static int active_color_index = -1;

static void update_world_section(application_t *application) {
    // Particle Settings Label
    nk_layout_row_dynamic(application->gui_context, DEFAULT_WIDGET_HEIGHT, 1);
    nk_label(application->gui_context, "PARTICLE SETTINGS", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    // Particle Count Label
    nk_layout_row_static(application->gui_context, 20, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(application->gui_context, "Particle Count", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    nk_labelf(application->gui_context, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM, "%d", application->tunables.particle_count);
    nk_layout_row_dynamic(application->gui_context, DEFAULT_WIDGET_HEIGHT, 1);
    if (nk_slider_int(application->gui_context, 0, (int *) &application->tunables.new_count, 50000, 1)) {
        application->tunables.dirty = true;
    }

    // Particle Types Label
    nk_layout_row_static(application->gui_context, 20, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(application->gui_context, "Particle Types", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    nk_labelf(application->gui_context, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM, "%d", application->tunables.nclass);
    nk_layout_row_dynamic(application->gui_context, DEFAULT_WIDGET_HEIGHT, 1);
    nk_slider_int(application->gui_context, 1, (int *) &application->tunables.nclass, 8, 1);

    // Particle Colors Label
    nk_layout_row_static(application->gui_context, 15, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(application->gui_context, "Particle Colors", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);
    // For each color type, display a circle with each corresponding color
    nk_layout_row_dynamic(application->gui_context, DEFAULT_WIDGET_HEIGHT, application->tunables.nclass);
    for (size_t i = 0; i < application->tunables.nclass; i++) {
        if (nk_button_color(application->gui_context, (struct nk_color) {
            .r = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[i][0]),
            .g = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[i][1]),
            .b = (nk_byte) (COLOR_RANGE * application->tunables.rgba_palette[i][2]),
            .a = (nk_byte) (COLOR_RANGE)
        } )) {
            active_color_index = (int) i;
        }
    }

    if (active_color_index >= 0) {
        if (nk_popup_begin(application->gui_context, NK_POPUP_STATIC, "Edit Color",
                            NK_WINDOW_CLOSABLE, nk_rect(DEFAULT_PANEL_WIDTH + 10, 50, 220, 260))) {
            nk_layout_row_dynamic(application->gui_context, 220, 1);
            nk_color_pick(application->gui_context, (struct nk_colorf *) application->tunables.rgba_palette[active_color_index], NK_RGBA);
            nk_popup_end(application->gui_context);
        } 
        else active_color_index = -1;
    }

    // combobox and randomize button
    static const float preset_row_ratio[] = { 0.7f, 0.3f };
    nk_layout_row(application->gui_context, NK_DYNAMIC, 25, 2, preset_row_ratio);
    if (nk_combo_begin_label(application->gui_context, "Presets",
                              nk_vec2(DEFAULT_PANEL_WIDTH - PANEL_CONTENT_RIGHT_PADDING, 200))) {
        nk_layout_row_dynamic(application->gui_context, 25, 1);
        for (int i = 0; i < COUNT; i++) {
            if (nk_combo_item_label(application->gui_context, preset_names[i], NK_TEXT_LEFT)) {
                memcpy(application->tunables.rgba_palette, color_presets[i], sizeof(application->tunables.rgba_palette));
            }
        }
        nk_combo_end(application->gui_context);
    }
    if (nk_button_label(application->gui_context, "Randomize")) {
        for (size_t i = 0; i < application->tunables.nclass; i++) {
            application->tunables.rgba_palette[i][0] = SDL_randf();
            application->tunables.rgba_palette[i][1] = SDL_randf();
            application->tunables.rgba_palette[i][2] = SDL_randf();
        }
    }
}

static void update_attraction_matrix_section(application_t *application) {
    // Attraction Matrix Section
    // for each color type, display a grid of all colors and their attraction factors
}

static void update_physics_section(application_t *application) {
    // Physics Control Section
    // Friction Half Life
    // Interaction Radius
    // Delta Time
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
 * @return             1 on success.
 *
 * @note Widget interactions (e.g. button presses) are handled inline here.
 * @see  init_gui(), mainloop()
 */
void update_gui(application_t *application) {
    if (nk_begin(application->gui_context, "Control Panel", nk_rect(0, 0, DEFAULT_PANEL_WIDTH, application->height), 
        NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE)
    ) {
        update_state_section(application);
        update_world_section(application);
        update_attraction_matrix_section(application);
        update_physics_section(application);
    }
    nk_end(application->gui_context);
}