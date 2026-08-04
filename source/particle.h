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

typedef struct particle {
    vector2D_t position;        ///< position of the particle (x, y)
    vector2D_t velocity;        ///< velocity of the particle (x, y)
} particle_t;

/// Inter-class attraction matrix (row-major, nclass x nclass).
typedef struct attraction {
    uint32_t length;    ///< Total entries in matrix (nclass * nclass).
    float *matrix;      ///< Attraction weights in [-1, 1], class i toward class j.
} attraction_t;

typedef struct particle_parameters {
    uint32_t particle_count;
    uint32_t new_count;
    uint32_t nclass;
    float rgba_palette[MAX_NUM_CLASSES][NUM_CHANNELS];
    float attraction_radius;
    float friction_halflife;
    float delta_time;
    bool dirty;
    bool shuffle;
} partparams_t;

#define MAX_PARTICLES       35000
#define RADIUS              1.5f
#define ATTRACTION_RADIUS   225.0f
#define FRICTION_HALFLIFE   2.0f
#define DELTATIME           0.1f

// init_particles : creates a pointer to an array of n particles and initializes the attraction matrix
bool init_particles(application_t *application, uint32_t n, uint8_t num_classes);

uint32_t recount_particles(application_t *application);

// shuffle_particles : Shuffles particle positions, reset velocities, and shuffles attraction matrix
void shuffle_particles(application_t *application);

// destroy_particles : destroys (frees) the particles array
bool destroy_particles(application_t *application);

#endif