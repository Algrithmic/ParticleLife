#ifndef PARTICLE_H
#define PARTICLE_H

#include "SDL_pixels.h"

typedef struct v2D {
    float x;
    float y;
} vector2D_t;

typedef struct application application_t;

#define NUMPARTICLES    1024
#define MAXNUMCLASSES   8

// index of each color class within color[]
typedef enum classifier {
    RED,
    BLUE,
    GREEN,
    YELLOW,
    PURPLE,
    ORANGE,
    CYAN,
    WHITE,
} class_t;

#define RGBA_WHITE  255, 255, 255, 255
#define RGBA_BLACK    0,   0,   0, 255

typedef struct particle {
    vector2D_t position;        // position of the particle (x, y)
    vector2D_t velocity;        // velocity of the particle (x, y)
    class_t class;              // class (color) of the particle (r, g, b, a)
} particle_t;

#define RADIUS      2
#define DIAMETER    (2 * RADIUS)

typedef struct attraction {
    unsigned int nclass;
    unsigned int length;
    float *matrix;
} attraction_t;

#define MAXDISTANCE         225.0f
#define FRICTIONHALFLIFE    2.0f
#define DELTATIME           0.075f

// init_particles : creates a pointer to an array of n particles and initializes the attraction matrix
particle_t *init_particles(application_t *application, unsigned int n, unsigned int num_classes);

// destroy_particles : destroys (frees) the particles array
void destroy_particles(particle_t *particles);

// update_particles : updates the particles array with new positions based on attraction factor
void update_particles(application_t *application, particle_t *particles);

// draw_particle : draws a particle to the screen
void draw_particle(application_t *application, particle_t particle);

#endif