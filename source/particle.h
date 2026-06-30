#ifndef PARTICLE_H
#define PARTICLE_H

#include "SDL_pixels.h"

typedef struct v2D {
    float x;
    float y;
} vector2D_t;

typedef struct application application_t;

#define NUM_PARTICLES   30000
#define MAX_NUM_CLASSES 8

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
#define RGBA_BLACK  0.0f, 0.0f, 0.0f, 1.0f

#define NUM_CHANNELS    4

extern float rgba[MAX_NUM_CLASSES][NUM_CHANNELS];

typedef struct particle {
    vector2D_t position;        // position of the particle (x, y)
    vector2D_t velocity;        // velocity of the particle (x, y)
    class_t class;              // class (color) of the particle
    uint32_t _padding;          // padding for CPU and GPU memory alignment
} particle_t;

#define RADIUS      2
#define DIAMETER    (2 * RADIUS)

typedef struct attraction {
    unsigned int nclass;
    unsigned int length;
    float *matrix;
} attraction_t;

#define MAXDISTANCE         225.0f
#define FRICTIONHALFLIFE    0.25f
#define DELTATIME           0.075f

// init_particles : creates a pointer to an array of n particles and initializes the attraction matrix
int init_particles(application_t *application, unsigned int n, unsigned int num_classes);

// destroy_particles : destroys (frees) the particles array
int destroy_particles(application_t *application);

#endif