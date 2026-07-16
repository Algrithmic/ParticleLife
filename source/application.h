/**
 * @file application.h
 * @brief Application state type and lifecycle API.
 *
 * Declares the central application_t state (window, GL/GUI contexts, simulation
 * data) and the functions that initialize, run, and destroy the application.
 */
#ifndef APPLICATION_H
#define APPLICATION_H

#include "particle.h"
#include "shaders.h"
#include "gui.h"

// forward declarations
typedef struct SDL_Window SDL_Window;
typedef struct nk_context nk_context_t;
typedef struct SDL_GLContextState *SDL_GLContext;

/// Lifecycle state of the application.
enum state {
    UNINITIALIZED = 0,  ///< Not yet initialized, or torn down / quitting.
    RUNNING,            ///< Actively simulating and rendering.
    PAUSED,             ///< Alive but simulation paused
};

/// Central application state shared across subsystems.
typedef struct application {
    SDL_Window *window;         ///< SDL window owning the OpenGL surface.
    SDL_GLContext gl_context;   ///< OpenGL rendering context.
    nk_context_t *gui_context;  ///< Nuklear GUI context.
    int32_t height;             ///< Current window height in pixels.
    int32_t width;              ///< Current window width in pixels.
    shader_t shader_data;       ///< Graphics/compute programs and vertex buffers.
    particle_t *particles;      ///< Heap-allocated array of NUM_PARTICLES particles.
    partparams_t tunables;      ///< Tunable particle parameters
    attraction_t attraction;    ///< Inter-class attraction matrix.
    enum state state;           ///< Current lifecycle state.
} application_t;

// init_application : initializes the resources for the application
int init_application(application_t *application);

// init_simulation : initializes the simulation particles and resources for the application
int init_simulation(application_t *application);

// destroy_application : destroys the allocated resources for the application
int destroy_application(application_t *application);

// mainloop : the application main loop
int mainloop(application_t *application);

#endif