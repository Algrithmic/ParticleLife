#include <stdio.h>

#include "SDL.h"
#include "SDL_video.h"
#include "glad/glad.h"

#include "../include/application.h"
#include "../include/particle.h"

#define DEFAULT_HEIGHT  800
#define DEFAULT_WIDTH   1800

// init_application : initializes the resources for the application
int init_application(application_t *application) {
    if (application->state != UNINITIALIZED) return 0;

    // Initialize SDL subsystems
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL Initialization failed. ERROR: %s\n", SDL_GetError());
        return 0;
    }

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
    application->renderer = SDL_CreateRenderer(application->window, NULL);
    if (application->renderer == NULL) {
        printf("Renderer Creation failed. ERROR: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    application->state = RUNNING;

    return 1;
}

// destroy_application : destroys the allocated resources for the application
int destroy_application(application_t *application) {
    if (application->state != RUNNING && application->state != PAUSED) return 0;

    if (application->renderer) {
        SDL_DestroyRenderer(application->renderer);
        application->renderer = NULL;
    }

    // Destroy Window
    if (application->window) {
        SDL_DestroyWindow(application->window);
        application->window = NULL;
    }
    
    // Quit subsystems
    SDL_Quit();

    return 1;
}


// handle_events : the event loop - checks for events and returns them
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

// mainloop : the application main loop
int mainloop(application_t *application) {
    particle_t *particles = init_particles(application, NUMPARTICLES, MAXNUMCLASSES);

    while (application->state == RUNNING || application->state == PAUSED) {
        handle_events(application);

        // Set draw color to black and clear
        SDL_SetRenderDrawColor(application->renderer, RGBA_BLACK);  // R, G, B, A
        SDL_RenderClear(application->renderer);

        // Draw particles on the screen
        for (int i = 0; i < NUMPARTICLES; i++)
            draw_particle(application, particles[i]);

        update_particles(application, particles);

        SDL_RenderPresent(application->renderer);
    }

    destroy_particles(particles);

    return 1;
}