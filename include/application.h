#ifndef APPLICATION_H
#define APPLICATION_H

// forward declarations
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;

enum state {
    UNINITIALIZED = 0,
    RUNNING,
    PAUSED
};

typedef struct application {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int height;
    int width;
    double delta_time;
    enum state state;
} application_t;

// init_application : initializes the resources for the application
int init_application(application_t *application);

// destroy_application : destroys the allocated resources for the application
int destroy_application(application_t *application);

// mainloop : the application main loop
int mainloop(application_t *application);

#endif