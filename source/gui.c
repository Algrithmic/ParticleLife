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
int init_gui(application_t *application) {
    application->gui_context = nk_sdl_init(application->window);
    if (application->gui_context == NULL)
        return 0;

    // Load default font
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    return 1;
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
int destroy_gui(void) {
    nk_sdl_shutdown();

    return 1;
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
int update_gui(application_t *application) {
    if (nk_begin(application->gui_context, "Test", nk_rect(50, 50, 230, 250), 
        NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE)
    ) {
        nk_layout_row_static(application->gui_context, 30, 80, 1);
        if (nk_button_label(application->gui_context, "button1"))
            printf("Button is pressed\n");
    }
    nk_end(application->gui_context);

    return 1;
}