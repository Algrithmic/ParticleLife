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

/// A single simulated particle. Its class is derived from its index, not stored.
typedef struct particle {
    vector2D_t position;        ///< position of the particle (x, y)
    vector2D_t velocity;        ///< velocity of the particle (x, y)
} particle_t;

/// Inter-class attraction matrix (row-major, nclass x nclass).
typedef struct attraction {
    uint32_t length;    ///< Total entries in matrix (nclass * nclass).
    float *matrix;      ///< Attraction weights in [-1, 1], class i toward class j.
} attraction_t;

/// User-tunable simulation parameters, edited via the GUI and read each frame.
typedef struct particle_parameters {
    uint32_t particle_count;    ///< Number of active particles currently simulated.
    uint32_t new_count;         ///< Pending count from the GUI slider; applied when dirty_count is set.
    uint32_t nclass;            ///< Number of active particle classes (1..MAX_NUM_CLASSES).
    float rgba_palette[MAX_NUM_CLASSES][NUM_CHANNELS]; ///< Per-class RGBA color, values in [0, 1].
    float attraction_radius;    ///< Maximum distance over which particles interact.
    float friction_halflife;    ///< Time for velocity to decay by half (velocity damping).
    float delta_time;           ///< Simulation time step per frame.
    bool dirty_count;           ///< Set when new_count differs and the particle buffer needs resizing.
    bool dirty_matrix;          ///< Set when the attraction matrix changed and must be re-uploaded.
    bool shuffle;               ///< Set to request a full randomized restart next frame.
} partparams_t;

#define MAX_PARTICLES       35000
#define RADIUS              2.0f
#define ATTRACTION_RADIUS   100.0f
#define FRICTION_HALFLIFE   2.0f
#define DELTATIME           0.075f

/// Allocate the particle array and attraction matrix and seed the tunables.
bool init_particles(application_t *application, uint32_t n, uint8_t num_classes);

/// Apply a pending particle-count change; returns the count of newly spawned particles.
uint32_t recount_particles(application_t *application);

/// Respawn all particles at random positions and re-randomize the attraction matrix.
void shuffle_particles(application_t *application);

/// Free the particle array and attraction matrix.
bool destroy_particles(application_t *application);

#endif