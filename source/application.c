#include <stdio.h>
#include <math.h>
#include <cglm/cglm.h>

#include "SDL.h"
#include "SDL_video.h"
#include "glad/glad.h"

#include "application.h"
#include "particle.h"
#include "shaders.h"

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
    application->delta_time = 0.0F;

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
    application->context = SDL_GL_CreateContext(application->window);
    if (application->context == NULL) {
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
    if (!init_particles(application, NUM_PARTICLES, 8)) {
        printf("Simulation Initialization failed.");
        return 0;
    }
    
    if (!init_graphics(&application->shader_data, application->particles,  2,  "./shaders/particle.vert",  "./shaders/particle.frag")) {
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

    // Particles SSBO
    uint32_t particle_ssbo;
    glGenBuffers(1, &particle_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particle_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(particle_t), application->particles, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particle_ssbo);   // binding = 0

    glBindVertexArray(application->shader_data.vao);
    glBindBuffer(GL_ARRAY_BUFFER, particle_ssbo);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(particle_t), (void *) offsetof(particle_t, position));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(particle_t), (void *) offsetof(particle_t, class));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);

    // Attraction Matrix SSBO
    uint32_t attraction_ssbo;
    glGenBuffers(1, &attraction_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, attraction_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, application->attraction.length * sizeof(float), application->attraction.matrix, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, attraction_ssbo); // binding = 1

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
 * @see init_application()
 */
int destroy_application(application_t *application) {
    if (application->state != RUNNING && application->state != PAUSED) return 0;

    if (application->context) 
        SDL_GL_DestroyContext(application->context);

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
 * viewport and projection matrix), and mouse events. Unrecognized events
 * are silently ignored.
 *
 * @param application  Pointer to the running application state.
 *
 * @note This is a static internal function and should only be called from mainloop().
 * @warning On quit, sets application state to UNINITIALIZED, which will cause
 *          mainloop() to exit on the next iteration.
 */
static void handle_events(application_t *application) {
    SDL_Event e;
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
    }
}

/**
 * mainloop
 *
 * @brief Runs the main application loop until the application exits.
 *
 * Each frame: polls events, clears the screen, dispatches the compute shader
 * for physics, then renders all particles via instanced drawing. Continues
 * until the application state is no longer RUNNING or PAUSED.
 *
 * @param application  Pointer to the running application state.
 * @return             1 when the loop exits cleanly.
 *
 * @note Expects both the graphics and compute shader programs to be initialized
 *       via init_simulation() before calling.
 * @see  handle_events(), init_simulation()
 */
int mainloop(application_t *application) {
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    while (application->state == RUNNING || application->state == PAUSED) {
        handle_events(application);
        
        // Set draw color to black and clear
        glClearColor(RGBA_BLACK);  // R, G, B, A
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Physics
        glUseProgram(application->shader_data.compute_program);
        glUniform1ui(glGetUniformLocation(application->shader_data.compute_program, "nclass"),      application->attraction.nclass);
        glUniform1ui(glGetUniformLocation(application->shader_data.compute_program, "count"),       NUM_PARTICLES);
        glUniform1f (glGetUniformLocation(application->shader_data.compute_program, "deltaTime"),   DELTATIME);
        glUniform1f (glGetUniformLocation(application->shader_data.compute_program, "friction"),    FRICTIONHALFLIFE);
        glUniform2f (glGetUniformLocation(application->shader_data.compute_program, "screenSize"),  (float)application->width, (float)application->height);
        glDispatchCompute((NUM_PARTICLES + 255) / 256, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        
        glUseProgram(application->shader_data.graphics_program);
        glBindVertexArray(application->shader_data.vao);
        glUniform4fv(glGetUniformLocation(application->shader_data.graphics_program, "palette"), application->attraction.nclass, &rgba[0][0]);
        glUniform1f(glGetUniformLocation(application->shader_data.graphics_program, "radius"), RADIUS);

        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, NUM_COORDINATES, NUM_PARTICLES);

        SDL_GL_SwapWindow(application->window);
    }

    return 1;
}

