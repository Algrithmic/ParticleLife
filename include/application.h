#ifndef APPLICATION_H
#define APPLICATION_H

#include "../include/shaders.h"

// forward declarations
typedef struct SDL_Window SDL_Window;

enum state {
    UNINITIALIZED = 0,
    RUNNING,
    PAUSED
};

typedef struct application {
    SDL_Window *window;
    SDL_GLContext context;
    int32_t height;
    int32_t width;
    int64_t delta_time;
    shader_t shader_data;
    particle_t *particles;
    enum state state;
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