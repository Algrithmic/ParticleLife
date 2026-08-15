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
 * init_tunables
 *
 * @brief Populates the tunable simulation parameters with their starting values.
 *
 * Seeds the particle count, class count, and physics tunables (attraction radius,
 * friction half-life, delta time) from the requested values and the compile-time
 * defaults, clears the dirty/shuffle flags, and loads the default RAINBOW color
 * palette.
 *
 * @param application  Pointer to the application whose tunables are initialized.
 * @param n            Initial particle count.
 * @param num_classes  Initial number of particle classes.
 * @return             1 on success.
 *
 * @note This is a static internal helper and should only be called from init_particles().
 */
static bool init_tunables(application_t *application, uint32_t n, uint8_t num_classes) {
    application->tunables.particle_count = n;
    application->tunables.new_count  = n;
    application->tunables.nclass     = num_classes;
    application->tunables.attraction_radius = ATTRACTION_RADIUS;
    application->tunables.friction_halflife = FRICTION_HALFLIFE;
    application->tunables.delta_time = DELTATIME;
    application->tunables.dirty_matrix = false;
    application->tunables.dirty_count  = false;
    application->tunables.shuffle = false;

    memcpy(application->tunables.rgba_palette, color_presets[RAINBOW], sizeof(color_presets[RAINBOW]));

    return true;
}

/**
 * init_particles
 *
 * @brief Allocates and initializes an array of n particles and the attraction matrix.
 *
 * Spawns each particle at a random position within the application window with
 * zero initial velocity. The particle array is allocated at MAX_PARTICLES so it
 * never has to be reallocated when the count grows at runtime. Also allocates the
 * application's attraction matrix and fills it with random weights in [-1, 1], and
 * seeds the tunable parameters via init_tunables().
 *
 * @param application  Pointer to the initialized application, used for window dimensions.
 * @param n            Number of particles to create.
 * @param num_classes  Number of particle classes (must not exceed MAX_NUM_CLASSES).
 * @returns 1 on success and 0 on failure
 *
 * @note A particle's class is not stored; it is derived at runtime from its index.
 * @see  destroy_particles(), init_tunables()
 */
bool init_particles(application_t *application, uint32_t n, uint8_t num_classes) {
    if (num_classes > MAX_NUM_CLASSES) {
        printf("particle.c : number of classes not supported\n");
        return false;
    }

    // initialize particles array at max size
    application->particles = (particle_t *) malloc(MAX_PARTICLES * sizeof(particle_t));
    if (application->particles == NULL) {
        printf("particle.c: Unable to allocate memory for particles");
        return false;
    }
    
    // Fill particle array
    for (uint32_t i = 0; i < n; i++) {
        application->particles[i] = new_particle(
            (vector2D_t) { .x = SDL_rand(application->width), .y = SDL_rand(application->height) },
            (vector2D_t) { .x = 0.0f, .y = 0.0f }
        );
    }

    // initializes the particle attraction matrix with random values [-1, 1]
    application->attraction.length = MAX_NUM_CLASSES * MAX_NUM_CLASSES;
    application->attraction.matrix = (float *) malloc((MAX_NUM_CLASSES * MAX_NUM_CLASSES) * sizeof(float));
    if (application->attraction.matrix == NULL) {
        printf("particle.c: Unable to allocate memory for attraction matrix");
        free(application->particles);
        return false;
    }
    
    // Fill attraction matrix | matrix[i] belongs to [-1.0f, 1.0f]
    for (uint32_t i = 0; i < application->attraction.length; i++)
        application->attraction.matrix[i] = SDL_randf() * 2.0f - 1.0f;

    (void) init_tunables(application, n, num_classes);

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
 * @param application  Pointer to the application whose particle count is updated.
 * @return             The number of newly spawned particles when growing, or 0 if
 *                     the count stayed the same or shrank (i.e. nothing new to upload).
 *
 * @note The caller uploads the newly spawned range to the GPU via update_particle_ssbo().
 * @see  update_particle_ssbo()
 */
uint32_t recount_particles(application_t *application) {
    uint32_t old_count = application->tunables.particle_count;
    uint32_t new_count = application->tunables.new_count;
    if (new_count > MAX_PARTICLES) new_count = MAX_PARTICLES;

    if (new_count > old_count) {
        for (uint32_t i = old_count; i < new_count; i++) {
            application->particles[i] = new_particle(
                (vector2D_t) { .x = SDL_rand(application->width), .y = SDL_rand(application->height) },
                (vector2D_t) { .x = 0.0f, .y = 0.0f }
            );
        }
    }

    application->tunables.particle_count = new_count;
    return (new_count > old_count) ? (new_count - old_count) : 0;
}

/**
 * shuffle_particles
 *
 * @brief Randomizes the simulation: respawns all particles and the attraction matrix.
 *
 * Re-scatters every active particle to a new random position with zero velocity,
 * and refills the entire attraction matrix with fresh random weights in [-1, 1].
 * Used to restart the simulation from a new random configuration.
 *
 * @param application  Pointer to the application to shuffle.
 *
 * @note The caller must re-upload both the particle and attraction buffers to the
 *       GPU afterwards (update_particle_ssbo(), update_attraction_ssbo()).
 * @see  update_particle_ssbo(), update_attraction_ssbo()
 */
void shuffle_particles(application_t *application) {
    // Shuffle Particle Parameters
    for (uint32_t i = 0; i < application->tunables.particle_count; i++) {
        application->particles[i] = new_particle(
            (vector2D_t) { .x = SDL_rand(application->width), .y = SDL_rand(application->height) },
            (vector2D_t) { .x = 0.0f, .y = 0.0f }
        );
    }
}

/**
 * destroy_particles
 *
 * @brief Frees the memory allocated for the particle array and attraction matrix.
 *
 * @param application  Pointer to the application state holding the particle
 *                     array and attraction matrix to free.
 * @returns 1 on success and 0 on failure
 *
 * @see init_particles()
 */
bool destroy_particles(application_t *application) {
    if (application->particles != NULL)
        free(application->particles);

    if (application->attraction.matrix != NULL) 
        free(application->attraction.matrix);

    return true;
}

