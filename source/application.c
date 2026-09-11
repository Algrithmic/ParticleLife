/**
 * @file application.c
 * @brief Application lifecycle, event handling, and the main render loop.
 *
 * Owns SDL/OpenGL setup and teardown, GPU resource creation for the simulation,
 * and the per-frame loop that drives input, the compute-shader physics step,
 * particle rendering, and the Nuklear GUI overlay.
 */
#include <stdio.h>
#include <math.h>
#include <cglm/cglm.h>

#include "SDL_video.h"

#include "glad/glad.h"

#include "application.h"
#include "particle.h"
#include "shaders.h"
#include "gui.h"


#define DEFAULT_HEIGHT  800
#define DEFAULT_WIDTH   1800

/**
 * init_application
 *
 * @brief Initializes the resources and subsystems required for the application.
 *
 * Sets up SDL, creates the window and OpenGL context, and loads OpenGL function
 * pointers via GLAD. Does nothing if the application is already initialized.
 *
 * @param application  Pointer to the application state to initialize.
 * @return             1 on success, 0 on failure.
 *
 * @note The application must be in the UNINITIALIZED state before calling this.
 *       On failure, all partially initialized resources are cleaned up.
 */
bool init_application(application_t *application) {
    if (application->state != UNINITIALIZED) return 0;

    contexts_t *ctxs = &application->contexts;

    // Initialize SDL subsystems
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL Initialization failed. ERROR: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    ctxs->screen.height = DEFAULT_HEIGHT;
    ctxs->screen.width  = DEFAULT_WIDTH;

    ctxs->camera.x    = 0.0f;
    ctxs->camera.y    = 0.0f;
    ctxs->camera.zoom = 1.0f;

    // initialize the window
    ctxs->screen.window = SDL_CreateWindow("Particle Life", DEFAULT_WIDTH, DEFAULT_HEIGHT, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (ctxs->screen.window == NULL) {
        printf("Window Creation failed. ERROR: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    SDL_GetWindowSizeInPixels(ctxs->screen.window, &ctxs->screen.width, &ctxs->screen.height);

    if (!SDL_SetWindowResizable(ctxs->screen.window, true)) {
        printf("Unable to set window resizable. ERROR: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctxs->screen.window);
        SDL_Quit();
        return false;
    }

    // Initialize the renderer
    ctxs->gl = SDL_GL_CreateContext(ctxs->screen.window);
    if (ctxs->gl == NULL) {
        printf("Renderer Creation failed. ERROR: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctxs->screen.window);
        SDL_Quit();
        return false;
    }

    // initialize GLAD in SDL for looking up OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD. ERROR: %s\n", SDL_GetError());
    }

    // Initialize text input to the application
    if (!SDL_StartTextInput(ctxs->screen.window)) {
        printf("Unable to start text input. ERROR: %s\n", SDL_GetError());
    }

    application->state = RUNNING;

    return true;
}

/**
 * build_projection
 *
 * @brief Builds the world-space-to-clip-space ortho matrix for the current camera.
 *
 * Maps the window's pixel rectangle to the world-space rectangle
 * [camera.x, camera.x + width/zoom] x [camera.y, camera.y + height/zoom], so
 * world coordinates render at true 1:1 scale when zoom == 1.0, panned by
 * camera.x/y and scaled by camera.zoom otherwise.
 *
 * @param screen  Current window/screen dimensions.
 * @param camera  Current pan/zoom camera state.
 * @param out     Output 4x4 matrix.
 */
static void build_projection(screen_t screen, camera_t camera, mat4 out) {
    float left   = camera.x;
    float right  = camera.x + (float) screen.width  / camera.zoom;
    float top    = camera.y;
    float bottom = camera.y + (float) screen.height / camera.zoom;
    glm_ortho(left, right, bottom, top, -1.0f, 1.0f, out);
}

#define STARTING_COUNT      1000
#define STARTING_CLASSES    4

/**
 * init_simulation
 *
 * @brief Initializes the simulation particles and GPU resources.
 *
 * Allocates and uploads particles to the GPU via SSBOs, sets up the graphics and
 * compute shader programs, configures vertex attributes, uploads the attraction
 * matrix, and builds the toroidal world-boundary outline program (see
 * init_border()/update_border_vbo() in shaders.c). The application window must
 * already be initialized before calling this function.
 *
 * @param application  Pointer to the initialized application state.
 * @return             1 on success, 0 if shader initialization fails.
 *
 * @note Calls init_particles() first, which populates application->attraction
 *       before it is uploaded to the GPU via init_buffers().
 * @see  init_application()
 */
bool init_simulation(application_t *application) {
    if (!init_particles(&application->world, STARTING_COUNT, STARTING_CLASSES)) {
        printf("Simulation Initialization failed.");
        return false;
    }

    if (!init_graphics(&application->shaders, &application->world, 2, "./shaders/particle.vert", "./shaders/particle.frag")) {
        printf("ERROR: Failed to initialize graphics shaders\n");
        return false;
    }

    if (!init_border(&application->shaders)) {
        printf("ERROR: Failed to initialize border shaders\n");
        return false;
    }
    update_border_vbo(&application->shaders,
        application->world.settings.world_width, application->world.settings.world_height);

    // Set the initial projection — the window size and camera are already known
    glUseProgram(application->shaders.graphics);
    mat4 projection = { 0 };
    build_projection(application->contexts.screen, application->contexts.camera, projection);
    glUniformMatrix4fv(
        glGetUniformLocation(application->shaders.graphics, "projection"),
        1, GL_FALSE, (float *) projection
    );

    // Compute Shader initialization
    if (!init_compute(&application->shaders, "./shaders/particle.comp")) {
        printf("ERROR: Failed to initialize compute shaders\n");
        return false;
    }

    // Shader Storage Buffer Objects
    if (!init_buffers(&application->shaders, &application->world)) {
        printf("ERROR: Failed to initialize shader storage buffer objects");
        return false;
    }

    return true;
}

/**
 * destroy_application
 *
 * @brief Destroys all resources allocated for the application and shuts down SDL.
 *
 * Destroys the OpenGL context, window, and particle data, then calls SDL_Quit.
 * Does nothing if the application is not in a RUNNING or PAUSED state.
 *
 * @param application  Pointer to the application state to destroy.
 * @return             1 on success, 0 if the application was not in a valid state.
 *
 * @note Tears down the Nuklear GUI first, while the OpenGL context is still
 *       current, before destroying the context and window.
 * @see init_application()
 */
bool destroy_application(application_t *application) {
    if (application->state != RUNNING && application->state != PAUSED) return 0;

    destroy_gui();

    if (application->contexts.gl) 
        SDL_GL_DestroyContext(application->contexts.gl);

    screen_t *screen = &application->contexts.screen;
    // Destroy Window
    if (screen->window) {
        SDL_DestroyWindow(screen->window);
        screen->window = NULL;
    }

    destroy_particles(&application->world);
    
    // Quit subsystems
    SDL_Quit();

    return true;
}


/**
 * handle_events
 *
 * @brief Polls and dispatches SDL events for the current frame.
 *
 * Handles window quit requests, window resize events (updating the OpenGL
 * viewport; the projection matrix itself is rebuilt every frame in
 * update_graphics() from the current screen size and camera), and mouse events
 * that drive the pan/zoom camera: left-drag pans (contexts->camera.x/y) and the
 * scroll wheel zooms (contexts->camera.zoom), anchored on the screen center so
 * the world point under it stays fixed. Both are skipped while the pointer is
 * over the Nuklear panel, so dragging/scrolling the GUI doesn't also move the
 * camera. Every event is also forwarded to Nuklear, wrapped in
 * nk_input_begin()/nk_input_end(), so the GUI can capture input. Unrecognized
 * events are silently ignored.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal function and should only be called from mainloop().
 * @warning On quit, sets application state to UNINITIALIZED, which will cause
 *          mainloop() to exit on the next iteration.
 */
static void handle_events(application_t *application) {
    contexts_t *ctxs = &application->contexts;
    SDL_Event e;
    nk_input_begin(ctxs->gui);
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT: {
                application->state = UNINITIALIZED;
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED: {
                SDL_GetWindowSizeInPixels(ctxs->screen.window, &ctxs->screen.width, &ctxs->screen.height);
                glViewport(0, 0, ctxs->screen.width, ctxs->screen.height);
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                if ((e.motion.state & SDL_BUTTON_LMASK) && !nk_window_is_any_hovered(ctxs->gui)) {
                    ctxs->camera.x -= e.motion.xrel / ctxs->camera.zoom;
                    ctxs->camera.y -= e.motion.yrel / ctxs->camera.zoom;
                }
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                if (nk_window_is_any_hovered(ctxs->gui)) break;

                float const zoom_step = 1.1f;
                float const min_zoom  = 0.1f;
                float const max_zoom  = 10.0f;
                float factor = (e.wheel.y > 0.0f) ? zoom_step : 1.0f / zoom_step;

                float old_zoom = ctxs->camera.zoom;
                float new_zoom = old_zoom * factor;
                if (new_zoom < min_zoom) new_zoom = min_zoom;
                if (new_zoom > max_zoom) new_zoom = max_zoom;

                // Keep the world point under the screen center fixed while zooming.
                float center_x = ctxs->camera.x + (ctxs->screen.width  / 2.0f) / old_zoom;
                float center_y = ctxs->camera.y + (ctxs->screen.height / 2.0f) / old_zoom;
                ctxs->camera.zoom = new_zoom;
                ctxs->camera.x = center_x - (ctxs->screen.width  / 2.0f) / new_zoom;
                ctxs->camera.y = center_y - (ctxs->screen.height / 2.0f) / new_zoom;
                break;
            }
            default: break;
        }

        nk_sdl_handle_event(&e);
    }
    nk_input_end(application->contexts.gui);
}

/**
 * update_physics
 *
 * @brief Runs one physics step for all particles on the GPU.
 *
 * Binds the compute program, uploads the simulation uniforms, dispatches one
 * compute workgroup per 256 particles, and issues a memory barrier so the
 * updated particle data is visible to the subsequent draw.
 *
 * @param shaders  Pointer to the shader state holding the compute program.
 * @param world    Pointer to the world whose tunables are uploaded.
 * @param dt       Simulation time step for this frame, in seconds: measured
 *                 real elapsed time scaled by the user's speed setting.
 *
 * @note This is a static internal function and should only be called from mainloop().
 * @see  update_graphics()
 */
static void update_physics(shader_t *shaders, world_t *world, float dt) {
    glUseProgram(shaders->compute);

    glUniform1ui(glGetUniformLocation(shaders->compute, "maxClass"),    MAX_NUM_CLASSES);
    glUniform1ui(glGetUniformLocation(shaders->compute, "nClass"),      world->settings.nclass);
    glUniform1ui(glGetUniformLocation(shaders->compute, "count"),       world->settings.particle_count);
    glUniform1f (glGetUniformLocation(shaders->compute, "deltaTime"),   dt);
    glUniform1f (glGetUniformLocation(shaders->compute, "frictionRate"),  world->settings.friction);
    glUniform1f (glGetUniformLocation(shaders->compute, "attractionRadius"),world->settings.attraction_radius);
    glUniform2f (glGetUniformLocation(shaders->compute, "worldSize"),
        world->settings.world_width, world->settings.world_height);
    glUniform1i (glGetUniformLocation(shaders->compute, "infinite"),
        world->settings.boundary_mode == BOUNDARY_INFINITE);

    glDispatchCompute((world->settings.particle_count + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

/**
 * update_graphics
 *
 * @brief Renders all particles, and the world-boundary outline, for the current frame.
 *
 * Rebuilds the world-to-clip-space projection from the current screen size and
 * camera (see build_projection()), binds the graphics program and VAO, uploads
 * the projection/palette/radius uniforms, and draws every particle in a single
 * instanced draw call. When boundary_mode is BOUNDARY_TOROIDAL, also draws the
 * white boundary rectangle via draw_border(), reusing the same projection.
 *
 * @param shaders  Pointer to the shader state holding the graphics/border programs.
 * @param world    Pointer to the world holding the particles, palette, and boundary mode.
 * @param screen   Current window/screen dimensions.
 * @param camera   Current pan/zoom camera state.
 *
 * @note This is a static internal function and should only be called from mainloop().
 * @see  update_physics(), build_projection(), draw_border()
 */
static void update_graphics(shader_t *shaders, world_t *world, screen_t screen, camera_t camera) {
    glUseProgram(shaders->graphics);
    glBindVertexArray(shaders->vao);

    mat4 projection = { 0 };
    build_projection(screen, camera, projection);
    glUniformMatrix4fv(
        glGetUniformLocation(shaders->graphics, "projection"),
        1, GL_FALSE, (float *) projection
    );

    glUniform4fv(glGetUniformLocation(shaders->graphics, "palette"), MAX_NUM_CLASSES, &world->settings.palette[0][0]);
    glUniform1f(glGetUniformLocation(shaders->graphics, "radius"), RADIUS);
    glUniform1ui(glGetUniformLocation(shaders->graphics, "nclass"), world->settings.nclass);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, NUM_COORDINATES, world->settings.particle_count);

    if (world->settings.boundary_mode == BOUNDARY_TOROIDAL)
        draw_border(shaders, (float *) projection);
}

/**
 * update_settings
 *
 * @brief Applies any pending GUI-driven changes to the simulation and its GPU buffers.
 *
 * Checks each dirty flag in world->settings and, if set, applies the pending
 * change and clears the flag:
 * - dirty_count: resizes the active particle count (recount_particles()) and
 *   uploads any newly spawned particles.
 * - dirty_matrix: re-uploads the full attraction matrix.
 * - dirty_world: applies the pending world_width/world_height, respawns all
 *   particles within the new bounds (respawn_particles_in_world()), re-uploads
 *   them, and refreshes the boundary-outline VBO (update_border_vbo()) so it
 *   matches the new size.
 * - shuffle: respawns all particles and re-uploads them. Does not touch the
 *   attraction matrix — that's a separate "Randomize" button that sets
 *   dirty_matrix directly (see update_attraction_matrix_section() in gui.c).
 *
 * @param shaders  Pointer to the shader state holding the SSBOs and border VBO.
 * @param world    Pointer to the world whose settings/particles are updated.
 *
 * @note This is a static internal function and should only be called from mainloop().
 */
static void update_settings(shader_t *shaders, world_t *world) {
    if (world->settings.dirty_count) {
        uint32_t old_count = world->settings.particle_count;
        uint32_t grown_count = recount_particles(world);
        if (grown_count > 0)
            update_particle_ssbo(shaders, world, old_count, grown_count);
        world->settings.dirty_count = false;
    }
    if (world->settings.dirty_matrix) {
        update_attraction_ssbo(shaders, world);
        world->settings.dirty_matrix = false;
    }
    if (world->settings.dirty_world) {
        world->settings.world_width  = world->settings.new_world_width;
        world->settings.world_height = world->settings.new_world_height;
        respawn_particles_in_world(world);
        update_particle_ssbo(
            shaders,
            world,
            0,
            world->settings.particle_count);
        update_border_vbo(shaders, world->settings.world_width, world->settings.world_height);
        world->settings.dirty_world = false;
    }
    if (world->settings.shuffle) {
        shuffle_particles(world);
        update_particle_ssbo(
            shaders, 
            world, 
            0, 
            world->settings.particle_count);
        world->settings.shuffle = false;
    }
}

#define RGBA_BLACK  0.0f, 0.0f, 0.0f, 1.0f
#define MAX_FRAME_TIME  (1.0f / 15.0f)  ///< Clamp on measured elapsed time, to avoid a sim jump after a stall.

/**
 * mainloop
 *
 * @brief Runs the main application loop until the application exits.
 *
 * Each frame: polls events (also feeding the GUI), builds the GUI layout, clears
 * the screen, dispatches the compute shader for physics, renders all particles
 * via instanced drawing, then draws the GUI overlay on top and presents the
 * frame. Continues until the application state is no longer RUNNING or PAUSED.
 *
 * @param application  Pointer to the running application state.
 * @return             1 when the loop exits cleanly.
 *
 * @note Expects the graphics and compute shader programs to be initialized via
 *       init_simulation(), and the GUI via init_gui(), before calling.
 * @note nk_sdl_render() both draws the GUI and closes out the Nuklear frame; it
 *       must run every iteration or the next frame's nk_begin() will assert.
 * @note The physics step is driven by measured wall-clock time (scaled by the
 *       user's speed setting), clamped to MAX_FRAME_TIME, so simulation speed
 *       stays consistent across machines and framerates rather than being tied
 *       to a fixed per-frame step.
 * @see  handle_events(), update_physics(), update_graphics(), init_simulation()
 */
bool mainloop(application_t *application) {
    uint64_t last_ticks = SDL_GetPerformanceCounter();
    uint64_t const ticks_per_second = SDL_GetPerformanceFrequency();

    while (application->state == RUNNING || application->state == PAUSED) {
        uint64_t const now_ticks = SDL_GetPerformanceCounter();
        float elapsed = (float) (now_ticks - last_ticks) / (float) ticks_per_second;
        last_ticks = now_ticks;
        if (elapsed > MAX_FRAME_TIME) elapsed = MAX_FRAME_TIME;

        handle_events(application);
        update_gui(application);
        update_settings(&application->shaders, &application->world);

        // Set draw color to black and clear
        glClearColor(RGBA_BLACK);
        glClear(GL_COLOR_BUFFER_BIT);

        if (application->state == RUNNING)
            update_physics(&application->shaders, &application->world,
                elapsed * application->world.settings.delta_time);
        update_graphics(&application->shaders, &application->world,
            application->contexts.screen, application->contexts.camera);

        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

        SDL_GL_SwapWindow(application->contexts.screen.window);
    }

    return true;
}

