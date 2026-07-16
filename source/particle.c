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

// color classes classified by class_t
float rgba[MAX_NUM_CLASSES][NUM_CHANNELS] = {
    { 1.00f, 0.23f, 0.19f, 1.0f },  // Red
    { 0.00f, 0.48f, 1.00f, 1.0f },  // Blue
    { 0.20f, 0.78f, 0.35f, 1.0f },  // Green
    { 1.00f, 0.80f, 0.00f, 1.0f },  // Yellow
    { 0.69f, 0.32f, 0.87f, 1.0f },  // Purple
    { 1.00f, 0.58f, 0.00f, 1.0f },  // Orange
    { 0.20f, 0.68f, 0.90f, 1.0f },  // Light Blue
    { 1.00f, 1.00f, 1.00f, 1.0f }   // White
};

/**
 * new_particle
 *
 * @brief Constructs and returns a new particle with the given position and class.
 *
 * @param position  The initial 2D position of the particle.
 * @param velocity  The initial velocity of the particle.
 * @param class     The class type that determines the particle's color and behavior.
 * @return          A newly constructed particle_t value.
 *
 * @note This is a static internal helper and should only be called from init_particles().
 */
static particle_t new_particle(vector2D_t position, vector2D_t velocity, class_t class) {
    return (particle_t) {
        .position = position,
        .velocity = velocity,
        .class = class
    };
}

/**
 * init_particles
 *
 * @brief Allocates and initializes an array of n particles and the attraction matrix.
 *
 * Spawns each particle at a random position within the application window, assigning
 * classes in round-robin order. Also allocates and populates the global attraction
 * matrix with random values in the range [-1, 1].
 *
 * @param application  Pointer to the initialized application, used for window dimensions.
 * @param n            Number of particles to create.
 * @param num_classes  Number of particle classes (must not exceed MAX_NUM_CLASSES).
 * @returns 1 on success and 0 on failure
 * 
 * @see  destroy_particles()
 */
int init_particles(application_t *application, unsigned int n, unsigned int num_classes) {
    if (num_classes > MAX_NUM_CLASSES) {
        printf("particle.c : number of classes not supported\n");
        return 0;
    }

    // initialize particles array
    application->particles = (particle_t *) malloc(n * sizeof(particle_t));
    if (application->particles == NULL) {
        printf("particle.c: Unable to allocate memory for particles");
        return 0;
    }
    
    // Fill particle array
    for (unsigned int i = 0; i < n; i++) {
        application->particles[i] = new_particle(
            (vector2D_t) { .x = SDL_rand(application->width), .y = SDL_rand(application->height) },
            (vector2D_t) { .x = 0.0f, .y = 0.0f },
            i % num_classes
        );
    }

    // initializes the particle attraction matrix with random values [-1, 1]
    application->attraction.nclass = num_classes;
    application->attraction.length = num_classes * num_classes;
    application->attraction.matrix = (float *) malloc((num_classes * num_classes) * sizeof(float));
    if (application->attraction.matrix == NULL) {
        printf("particle.c: Unable to allocate memory for attraction matrix");

        free(application->particles);
        return 0;
    }

    // Fill attraction matrix | matrix[i] belongs to [-1.0f, 1.0f]
    for (unsigned int i = 0; i < application->attraction.length; i++)
        application->attraction.matrix[i] = SDL_randf() * 2.0f - 1.0f;
    
    
    return 1;
}

void shuffle_particles(application_t *application) {
    // Shuffle Particle Parameters
    for (uint32_t i = 0; i < application->tunables.particle_count; i++) {
        application->particles[i].position.x = SDL_rand(application->width);
        application->particles[i].position.y = SDL_rand(application->height);
        application->particles[i].velocity.x = 0.0f;
        application->particles[i].velocity.y = 0.0f;
    }

    // Fill attraction matrix | matrix[i] belongs to [-1.0f, 1.0f]
    for (unsigned int i = 0; i < application->attraction.length; i++)
        application->attraction.matrix[i] = SDL_randf() * 2.0f - 1.0f;
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
int destroy_particles(application_t *application) {
    if (application->particles != NULL)
        free(application->particles);

    if (application->attraction.matrix != NULL) 
        free(application->attraction.matrix);

    return 1;
}

