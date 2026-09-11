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
typedef struct world world_t;
typedef struct screen screen_t;

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

/// World boundary behavior: a bounded, wrapping domain or an unbounded plane.
typedef enum boundary_mode {
    BOUNDARY_TOROIDAL,  ///< Particles wrap around a world_width x world_height domain.
    BOUNDARY_INFINITE,  ///< No wrapping; particles may drift anywhere on the plane.
} boundary_mode_t;

/// User-tunable simulation parameters, edited via the GUI and read each frame.
typedef struct settings {
    uint32_t particle_count;    ///< Number of active particles currently simulated.
    uint32_t new_count;         ///< Pending count from the GUI slider; applied when dirty_count is set.
    uint32_t nclass;            ///< Number of active particle classes (1..MAX_NUM_CLASSES).
    float palette[MAX_NUM_CLASSES][NUM_CHANNELS]; ///< Per-class RGBA color, values in [0, 1].
    float attraction_radius;    ///< Maximum distance over which particles interact.
    float friction;             ///< Velocity decay rate, in 1/seconds; 0 = no damping, higher = more damping.
    float delta_time;           ///< Speed multiplier applied to the measured real elapsed time each frame.
    float world_width;          ///< Current applied toroidal world width, in world units.
    float world_height;         ///< Current applied toroidal world height, in world units.
    float new_world_width;      ///< Pending width from the GUI slider; applied when dirty_world is set.
    float new_world_height;     ///< Pending height from the GUI slider; applied when dirty_world is set.
    boundary_mode_t boundary_mode; ///< Toroidal (bounded, wrapping) or infinite (unbounded).
    bool dirty_count;           ///< Set when new_count differs and the particle buffer needs resizing.
    bool dirty_matrix;          ///< Set when the attraction matrix changed and must be re-uploaded.
    bool dirty_world;           ///< Set when world size or boundary_mode changed and must be applied.
    bool shuffle;               ///< Set to request a full randomized restart next frame.
} settings_t;

#define MAX_PARTICLES       35000
#define RADIUS              3.0f
#define ATTRACTION_RADIUS   100.0f
#define FRICTION            0.5f   ///< Default decay rate, in 1/seconds (half-life of 1/0.5 = 2s).
#define DELTATIME           4.5f   ///< Default speed multiplier; matches the old fixed dt's feel at 60fps.
#define DEFAULT_WORLD_WIDTH     1800.0f
#define DEFAULT_WORLD_HEIGHT    800.0f
#define MAX_WORLD_SIZE          3000.0f

/// Allocate the particle array and attraction matrix and seed the tunables.
bool init_particles(world_t *world, uint32_t n, uint8_t num_classes);

/// Apply a pending particle-count change; returns the count of newly spawned particles.
uint32_t recount_particles(world_t *world);

/// Respawn all particles at random positions and re-randomize the attraction matrix.
void shuffle_particles(world_t *world);

/// Re-scatter all particles at random positions within the current world bounds (no matrix change).
void respawn_particles_in_world(world_t *world);

/// Free the particle array and attraction matrix.
bool destroy_particles(world_t *world);

#endif