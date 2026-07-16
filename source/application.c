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

#include "SDL.h"
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
int init_application(application_t *application) {
    if (application->state != UNINITIALIZED) return 0;

    // Initialize SDL subsystems
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL Initialization failed. ERROR: %s\n", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    application->height = DEFAULT_HEIGHT;
    application->width = DEFAULT_WIDTH;

    // initialize the window
    application->window = SDL_CreateWindow("Particle Life", DEFAULT_WIDTH, DEFAULT_HEIGHT, SDL_WINDOW_OPENGL);
    if (application->window == NULL) {
        printf("Window Creation failed. ERROR: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    if (!SDL_SetWindowResizable(application->window, true)) {
        printf("Unable to set window resizable. ERROR: %s\n", SDL_GetError());
        SDL_DestroyWindow(application->window);
        SDL_Quit();
        return 0;
    }

    // Initialize the renderer
    application->gl_context = SDL_GL_CreateContext(application->window);
    if (application->gl_context == NULL) {
        printf("Renderer Creation failed. ERROR: %s\n", SDL_GetError());
        SDL_DestroyWindow(application->window);
        SDL_Quit();
        return 0;
    }

    // initialize GLAD in SDL for looking up OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD. ERROR: %s\n", SDL_GetError());
    }
    
    application->state = RUNNING;

    return 1;
}

#define ATTRACTION_RADIUS   225.0f
#define FRICTION_HALFLIFE   2.0f
#define DELTATIME           0.1f
#define STARTING_COUNT      1000
#define STARTING_CLASSES    4

/**
 * init_simulation
 *
 * @brief Initializes the simulation particles and GPU resources.
 *
 * Allocates and uploads particles to the GPU via SSBOs, sets up graphics and
 * compute shader programs, configures vertex attributes, and uploads the
 * attraction matrix. The application window must already be initialized before
 * calling this function.
 *
 * @param application  Pointer to the initialized application state.
 * @return             1 on success, 0 if shader initialization fails.
 *
 * @note Relies on the global `attraction` matrix being populated before call.
 * @see  init_application()
 */
int init_simulation(application_t *application) {
    if (!init_particles(application, STARTING_COUNT, STARTING_CLASSES)) {
        printf("Simulation Initialization failed.");
        return 0;
    }
    
    application->tunables.particle_count = STARTING_COUNT;
    application->tunables.nclass = STARTING_CLASSES;
    application->tunables.attraction_radius = ATTRACTION_RADIUS;
    application->tunables.friction_halflife = FRICTION_HALFLIFE;
    application->tunables.delta_time = DELTATIME;
    application->tunables.dirty = false;
    application->tunables.shuffle = false;

    if (!init_graphics(application,  2,  "./shaders/particle.vert",  "./shaders/particle.frag")) {
        printf("ERROR: Failed to initialize graphics shaders\n");
        return 0;
    }

    // Set the initial projection — the window size is already known
    glUseProgram(application->shader_data.graphics_program);
    mat4 projection;
    glm_ortho(0.0f, (float) application->width, (float) application->height, 0.0f, -1.0f, 1.0f, projection);
    glUniformMatrix4fv(
        glGetUniformLocation(application->shader_data.graphics_program, "projection"),
        1, GL_FALSE, (float *) projection
    );

    // Compute Shader initialization
    if (!init_compute(&application->shader_data, "./shaders/particle.comp")) {
        printf("ERROR: Failed to initialize compute shaders\n");
        return 0;
    }

    // Shader Storage Buffer Objects
    if (!init_buffers(application)) {
        printf("ERROR: Failed to initialize shader storage buffer objects");
        return 0;
    }

    return 1;
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
int destroy_application(application_t *application) {
    if (application->state != RUNNING && application->state != PAUSED) return 0;

    destroy_gui();

    if (application->gl_context) 
        SDL_GL_DestroyContext(application->gl_context);

    // Destroy Window
    if (application->window) {
        SDL_DestroyWindow(application->window);
        application->window = NULL;
    }

    destroy_particles(application);
    
    // Quit subsystems
    SDL_Quit();

    return 1;
}


/**
 * handle_events
 *
 * @brief Polls and dispatches SDL events for the current frame.
 *
 * Handles window quit requests, window resize events (updating the OpenGL
 * viewport and projection matrix), and mouse events. Every event is also
 * forwarded to Nuklear, wrapped in nk_input_begin()/nk_input_end(), so the GUI
 * can capture input. Unrecognized events are silently ignored.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal function and should only be called from mainloop().
 * @warning On quit, sets application state to UNINITIALIZED, which will cause
 *          mainloop() to exit on the next iteration.
 */
static void handle_events(application_t *application) {
    SDL_Event e;
    nk_input_begin(application->gui_context);
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT: {
                application->state = UNINITIALIZED;
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED: {
                SDL_GetWindowSize(application->window, &application->width, &application->height);
                glViewport(0, 0, application->width, application->height);

                mat4 projection;
                glm_ortho(0.0f, (float) application->width, (float) application->height, 0.0f, -1.0f, 1.0f, projection);
                glUniformMatrix4fv(
                    glGetUniformLocation(application->shader_data.graphics_program, "projection"),
                    1, GL_FALSE, (float *) projection
                );
                break;
            }
            case SDL_EVENT_MOUSE_MOTION:
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                break; 
            default: break;
        }

        nk_sdl_handle_event(&e);
    }
    nk_input_end(application->gui_context);
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
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal function and should only be called from mainloop().
 * @see  update_graphics()
 */
static void update_physics(application_t *application) {
    glUseProgram(application->shader_data.compute_program);
    glUniform1ui(glGetUniformLocation(application->shader_data.compute_program, "nclass"),      application->attraction.nclass);
    glUniform1ui(glGetUniformLocation(application->shader_data.compute_program, "count"),       application->tunables.particle_count);
    glUniform1f (glGetUniformLocation(application->shader_data.compute_program, "deltaTime"),   application->tunables.delta_time);
    glUniform1f (glGetUniformLocation(application->shader_data.compute_program, "friction"),    application->tunables.friction_halflife);
    glUniform1f (glGetUniformLocation(application->shader_data.compute_program, "attractionRadius"), application->tunables.attraction_radius);
    glUniform2f (glGetUniformLocation(application->shader_data.compute_program, "screenSize"),  (float)application->width, (float)application->height);
    glDispatchCompute((application->tunables.particle_count + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

/**
 * update_graphics
 *
 * @brief Renders all particles for the current frame via instanced drawing.
 *
 * Binds the graphics program and VAO, uploads the color palette and radius
 * uniforms, and draws every particle in a single instanced draw call.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal function and should only be called from mainloop().
 * @see  update_physics()
 */
static void update_graphics(application_t *application) {
    glUseProgram(application->shader_data.graphics_program);
    glBindVertexArray(application->shader_data.vao);
    glUniform4fv(glGetUniformLocation(application->shader_data.graphics_program, "palette"), application->attraction.nclass, &rgba[0][0]);
    glUniform1f(glGetUniformLocation(application->shader_data.graphics_program, "radius"), RADIUS);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, NUM_COORDINATES, application->tunables.particle_count);
}

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
 * @see  handle_events(), update_physics(), update_graphics(), init_simulation()
 */
int mainloop(application_t *application) {
    while (application->state == RUNNING || application->state == PAUSED) {
        handle_events(application);
        update_gui(application);
        if (application->tunables.shuffle) {
            shuffle_particles(application);
            update_particle_buffer(application);
            update_attraction_buffer(application);
            application->tunables.shuffle = false;
        }

        // Set draw color to black and clear
        glClearColor(RGBA_BLACK);  // R, G, B, A
        glClear(GL_COLOR_BUFFER_BIT);
        
        if (application->state == RUNNING)
            update_physics(application);
        update_graphics(application);

        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

        SDL_GL_SwapWindow(application->window);
    }

    return 1;
}

