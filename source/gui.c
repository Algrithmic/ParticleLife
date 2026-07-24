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

#include "application.h"
#include "gui.h"

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

static void update_world_section(application_t *application) {
    // Particle Settings Label
    nk_layout_row_dynamic(application->gui_context, DEFAULT_WIDGET_HEIGHT, 1);
    nk_label(application->gui_context, "PARTICLE SETTINGS", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    // Particle Count Label
    nk_layout_row_static(application->gui_context, 20, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(application->gui_context, "Particle Count", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    char particle_count[MAX_NUM_COUNT] = { '\0' };
    nk_itoa(particle_count, application->tunables.particle_count);
    nk_label(application->gui_context, particle_count, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM);
    nk_layout_row_dynamic(application->gui_context, DEFAULT_WIDGET_HEIGHT, 1);
    if (nk_slider_int(application->gui_context, 0, (int *) &application->tunables.new_count, 50000, 1)) {
        application->tunables.dirty = true; ///<---
    }

    // Particle Types Label
    nk_layout_row_static(application->gui_context, 20, DEFAULT_PANEL_WIDTH / 2 - PANEL_CONTENT_RIGHT_PADDING, 2);
    nk_label(application->gui_context, "Particle Types", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM);

    char class_count[MAX_NUM_COUNT] = { '\0' };
    nk_itoa(class_count, application->tunables.nclass);
    nk_label(application->gui_context, class_count, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_BOTTOM);
    nk_layout_row_dynamic(application->gui_context, DEFAULT_WIDGET_HEIGHT, 1);
    if (nk_slider_int(application->gui_context, 1, (int *) &application->tunables.nclass, 8, 1)) {
        // do nothing
    }

    // Particle Colors
    // For each color type, display a circle with each corresponding color
    // Below add a combobox with color presets
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