/**
 * @file application.h
 * @brief Application state type and lifecycle API.
 *
 * Declares the central application_t state (window, GL/GUI contexts, simulation
 * data) and the functions that initialize, run, and destroy the application.
 */
#ifndef APPLICATION_H
#define APPLICATION_H

#include "SDL.h"

#include "particle.h"
#include "shaders.h"
#include "gui.h"

// forward declarations
typedef struct SDL_Window SDL_Window;
typedef struct nk_context nk_context_t;
typedef struct SDL_GLContextState *SDL_GLContext;

/// Lifecycle state of the application.
enum state {
    UNINITIALIZED,      ///< Not yet initialized, or torn down / quitting.
    RUNNING,            ///< Actively simulating and rendering.
    PAUSED,             ///< Alive but simulation paused
};

typedef struct screen {
    SDL_Window *window;
    int32_t     height;
    int32_t     width;
} screen_t;

/// Pan/zoom view into world space; maps world units to window pixels.
typedef struct camera {
    float x;        ///< World-space X coordinate shown at the window's left edge.
    float y;        ///< World-space Y coordinate shown at the window's top edge.
    float zoom;      ///< Scale factor from world units to pixels (1.0 = 1:1).
} camera_t;

typedef struct contexts {
    screen_t      screen;
    camera_t      camera;
    SDL_GLContext gl;
    nk_context_t *gui;
} contexts_t;

typedef struct world {
    particle_t  *particles;     ///< Heap-allocated array of MAX_PARTICLES particles.
    settings_t   settings;      ///< Tunable particle parameters
    attraction_t attraction;    ///< Inter-class attraction matrix.
} world_t;

/// Central application state shared across subsystems.
typedef struct application {
    contexts_t   contexts;
    shader_t     shaders;       ///< Graphics/compute programs and vertex buffers.
    world_t      world;
    enum state   state;         ///< Current lifecycle state.
} application_t;

// init_application : initializes the resources for the application
bool init_application(application_t *application);

// init_simulation : initializes the simulation particles and resources for the application
bool init_simulation(application_t *application);

// destroy_application : destroys the allocated resources for the application
bool destroy_application(application_t *application);

// mainloop : the application main loop
bool mainloop(application_t *application);

#endif