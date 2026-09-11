/**
 * @file particle.c
 * @brief Particle and attraction-matrix allocation, initialization, and teardown.
 *
 * Defines the color palette shared with the renderer and the CPU-side setup of
 * the particle array and the inter-class attraction matrix that drives the
 * simulation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "SDL.h"

#include "particle.h"
#include "application.h"
#include "presets.h"

/**
 * new_particle
 *
 * @brief Constructs and returns a new particle with the given position and velocity.
 *
 * A particle stores only position and velocity; its class is derived at runtime
 * from its index (index % nclass) by the shaders, so it is not stored here.
 *
 * @param position  The initial 2D position of the particle.
 * @param velocity  The initial velocity of the particle.
 * @return          A newly constructed particle_t value.
 *
 * @note This is a static internal helper, called from init_particles(),
 *       recount_particles(), and shuffle_particles().
 */
static particle_t new_particle(vector2D_t position, vector2D_t velocity) {
    return (particle_t) {
        .position = position,
        .velocity = velocity
    };
}

/**
 * init_settings
 *
 * @brief Populates the tunable simulation parameters with their starting values.
 *
 * Seeds the particle count, class count, physics tunables (attraction radius,
 * friction decay rate, speed multiplier), and the default toroidal world size
 * and boundary mode from the requested values and the compile-time defaults,
 * clears the dirty/shuffle flags, and loads the default RAINBOW color palette.
 *
 * @param world        Pointer to the world whose tunables are initialized.
 * @param n            Initial particle count.
 * @param num_classes  Initial number of particle classes.
 * @return             1 on success.
 *
 * @note This is a static internal helper and should only be called from init_particles().
 */
static bool init_settings(world_t *world, uint32_t n, uint8_t num_classes) {
    world->settings.particle_count = n;
    world->settings.new_count  = n;
    world->settings.nclass     = num_classes;
    world->settings.attraction_radius = ATTRACTION_RADIUS;
    world->settings.friction = FRICTION;
    world->settings.delta_time = DELTATIME;
    world->settings.world_width  = DEFAULT_WORLD_WIDTH;
    world->settings.world_height = DEFAULT_WORLD_HEIGHT;
    world->settings.new_world_width  = DEFAULT_WORLD_WIDTH;
    world->settings.new_world_height = DEFAULT_WORLD_HEIGHT;
    world->settings.boundary_mode = BOUNDARY_INFINITE;
    world->settings.dirty_matrix = false;
    world->settings.dirty_count  = false;
    world->settings.dirty_world  = false;
    world->settings.shuffle = false;

    memcpy(world->settings.palette, color_presets[RAINBOW], sizeof(color_presets[RAINBOW]));

    return true;
}

/**
 * init_particles
 *
 * @brief Allocates and initializes an array of n particles and the attraction matrix.
 *
 * Seeds the tunable settings first (init_settings(), which sets world_width/
 * world_height to their compile-time defaults), then spawns each particle at a
 * random position within those world bounds with zero initial velocity. The
 * particle array is allocated at MAX_PARTICLES so it never has to be
 * reallocated when the count grows at runtime. Also allocates the world's
 * attraction matrix and fills it with random weights in [-1, 1].
 *
 * @param world        Pointer to the world to initialize.
 * @param n            Number of particles to create.
 * @param num_classes  Number of particle classes (must not exceed MAX_NUM_CLASSES).
 * @returns 1 on success and 0 on failure
 *
 * @note A particle's class is not stored; it is derived at runtime from its index.
 * @note The spawn loop must run after init_settings(), since it reads
 *       world->settings.world_width/world_height for the spawn bounds.
 * @see  destroy_particles(), init_settings()
 */
bool init_particles(world_t *world, uint32_t n, uint8_t num_classes) {
    if (num_classes > MAX_NUM_CLASSES) {
        printf("particle.c : number of classes not supported\n");
        return false;
    }

    // initialize particles array at max size
    world->particles = (particle_t *) malloc(MAX_PARTICLES * sizeof(particle_t));
    if (world->particles == NULL) {
        printf("particle.c: Unable to allocate memory for particles");
        return false;
    }

    // Seed the tunables (including world_width/world_height) before spawning,
    // since spawn positions are drawn from the world bounds.
    (void) init_settings(world, n, num_classes);

    // Fill particle array
    for (uint32_t i = 0; i < n; i++) {
        world->particles[i] = new_particle(
            (vector2D_t) { .x = SDL_rand((int) world->settings.world_width), .y = SDL_rand((int) world->settings.world_height) },
            (vector2D_t) { .x = 0.0f, .y = 0.0f }
        );
    }

    // initializes the particle attraction matrix with random values [-1, 1]
    world->attraction.length = MAX_NUM_CLASSES * MAX_NUM_CLASSES;
    world->attraction.matrix = (float *) malloc((MAX_NUM_CLASSES * MAX_NUM_CLASSES) * sizeof(float));
    if (world->attraction.matrix == NULL) {
        printf("particle.c: Unable to allocate memory for attraction matrix");
        free(world->particles);
        return false;
    }

    // Fill attraction matrix | matrix[i] belongs to [-1.0f, 1.0f]
    for (uint32_t i = 0; i < world->attraction.length; i++)
        world->attraction.matrix[i] = SDL_randf() * 2.0f - 1.0f;

    return true;
}

/**
 * recount_particles
 *
 * @brief Applies a requested change in particle count, spawning any new particles.
 *
 * Clamps the requested new_count to MAX_PARTICLES and updates the active particle
 * count. When the count grows, the newly activated slots are populated with fresh
 * particles at random positions; when it shrinks, the surplus particles are simply
 * left inactive (no reallocation occurs). Shrinking requires no buffer update.
 *
 * @param world  Pointer to the world whose particle count is updated.
 * @return       The number of newly spawned particles when growing, or 0 if
 *                     the count stayed the same or shrank (i.e. nothing new to upload).
 *
 * @note The caller uploads the newly spawned range to the GPU via update_particle_ssbo().
 * @see  update_particle_ssbo()
 */
uint32_t recount_particles(world_t *world) {
    uint32_t old_count = world->settings.particle_count;
    uint32_t new_count = world->settings.new_count;
    if (new_count > MAX_PARTICLES) new_count = MAX_PARTICLES;

    if (new_count > old_count) {
        for (uint32_t i = old_count; i < new_count; i++) {
            world->particles[i] = new_particle(
                (vector2D_t) { .x = SDL_rand((int) world->settings.world_width), .y = SDL_rand((int) world->settings.world_height) },
                (vector2D_t) { .x = 0.0f, .y = 0.0f }
            );
        }
    }

    world->settings.particle_count = new_count;
    return (new_count > old_count) ? (new_count - old_count) : 0;
}

/**
 * shuffle_particles
 *
 * @brief Re-scatters every active particle to a new random position with zero velocity.
 *
 * Does not touch the attraction matrix; that is randomized independently via
 * the GUI's "Randomize" button in the Attraction Matrix section, which raises
 * dirty_matrix on its own.
 *
 * @param world  Pointer to the world whose particles are respawned.
 *
 * @note The caller must re-upload the particle buffer to the GPU afterwards
 *       (update_particle_ssbo()).
 * @see  update_particle_ssbo()
 */
void shuffle_particles(world_t *world) {
    // Shuffle Particle Parameters
    for (uint32_t i = 0; i < world->settings.particle_count; i++) {
        world->particles[i] = new_particle(
            (vector2D_t) { .x = SDL_rand((int) world->settings.world_width), .y = SDL_rand((int) world->settings.world_height) },
            (vector2D_t) { .x = 0.0f, .y = 0.0f }
        );
    }
}

/**
 * respawn_particles_in_world
 *
 * @brief Re-scatters all active particles at random positions within the current world bounds.
 *
 * Unlike shuffle_particles(), the attraction matrix is left untouched. Used when
 * the world size or boundary mode changes, so existing particle positions remain
 * valid within the new bounds.
 *
 * @param world  Pointer to the world whose particles are respawned.
 */
void respawn_particles_in_world(world_t *world) {
    for (uint32_t i = 0; i < world->settings.particle_count; i++) {
        world->particles[i].position = (vector2D_t) {
            .x = SDL_rand((int) world->settings.world_width),
            .y = SDL_rand((int) world->settings.world_height)
        };
        world->particles[i].velocity = (vector2D_t) { .x = 0.0f, .y = 0.0f };
    }
}

/**
 * destroy_particles
 *
 * @brief Frees the memory allocated for the particle array and attraction matrix.
 *
 * @param world  Pointer to the world holding the particle array and attraction
 *               matrix to free.
 * @returns 1 on success and 0 on failure
 *
 * @see init_particles()
 */
bool destroy_particles(world_t *world) {
    if (world->particles != NULL)
        free(world->particles);

    if (world->attraction.matrix != NULL) 
        free(world->attraction.matrix);

    return true;
}

