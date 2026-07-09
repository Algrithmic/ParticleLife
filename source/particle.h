/**
 * @file particle.h
 * @brief Particle, class, and attraction-matrix types plus simulation constants.
 *
 * Defines the particle representation shared between CPU and GPU, the color
 * class enum, the attraction matrix type, and the tunable simulation constants.
 */
#ifndef PARTICLE_H
#define PARTICLE_H

#include "SDL_pixels.h"

/// Two-dimensional float vector (position or velocity).
typedef struct v2D {
    float x;
    float y;
} vector2D_t;

typedef struct application application_t;

#define NUM_PARTICLES   1000
#define MAX_NUM_CLASSES 8

/// Particle class; also the row index of its color within rgba[].
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

#define RGBA_BLACK  0.0f, 0.0f, 0.0f, 1.0f

#define NUM_CHANNELS    4

extern float rgba[MAX_NUM_CLASSES][NUM_CHANNELS];

typedef struct particle {
    vector2D_t position;        // position of the particle (x, y)
    vector2D_t velocity;        // velocity of the particle (x, y)
    class_t class;              // class (color) of the particle
    uint32_t _padding;          // padding for CPU and GPU memory alignment
} particle_t;


/// Inter-class attraction matrix (row-major, nclass x nclass).
typedef struct attraction {
    unsigned int nclass;    ///< Number of particle classes.
    unsigned int length;    ///< Total entries in matrix (nclass * nclass).
    float *matrix;          ///< Attraction weights in [-1, 1], class i toward class j.
} attraction_t;

#define RADIUS              3.0f
#define MAXDISTANCE         225.0f
#define FRICTIONHALFLIFE    2.0f
#define DELTATIME           0.1f

// init_particles : creates a pointer to an array of n particles and initializes the attraction matrix
int init_particles(application_t *application, unsigned int n, unsigned int num_classes);

// destroy_particles : destroys (frees) the particles array
int destroy_particles(application_t *application);

#endif