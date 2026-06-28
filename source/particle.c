#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "SDL.h"

#include "../include/particle.h"
#include "../include/application.h"

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

// new_particle : creates a new particle
static particle_t new_particle(vector2D_t position, class_t class) {
    return (particle_t) {
        .position = position,
        // .velocity = velocity,
        .class = class
    };
}

// particle attraction matrix used for physics calculations
attraction_t attraction;

// init_particles : creates a pointer to an array of n particles
particle_t *init_particles(application_t *application, unsigned int n, unsigned int num_classes) {
    if (num_classes > MAX_NUM_CLASSES) {
        printf("particle.c : number of classes not supported\n");
        return NULL;
    }

    // initialize particles array
    particle_t *particles = (particle_t *) malloc(n * sizeof(particle_t));
    for (unsigned int i = 0; i < n; i++) {
        particles[i] = new_particle(
            (vector2D_t) { .x = SDL_rand(application->width), .y = SDL_rand(application->height) },
            // (vector2D_t) { .x = 0.0f, .y = 0.0f },
            i % num_classes
        );
    }

    // initializes the particle attraction matrix with random values [-1, 1]
    attraction.nclass = num_classes;
    attraction.length = num_classes * num_classes;
    attraction.matrix = (float *) malloc((num_classes * num_classes) * sizeof(float));
    for (unsigned int i = 0; i < attraction.length; i++)
        attraction.matrix[i] = SDL_randf() * 2.0f - 1.0f;
        
    
    return particles;
}

// destroy_particles : destroys (frees) the particles array
void destroy_particles(particle_t *particles) {
    free(particles);
}

// alpha : gets the attraction value how much c1 is attracted to c2
// static float alpha(class_t c1, class_t c2) {
//     return attraction.matrix[c1 * attraction.nclass + c2];
// }

// // force : determines the force applied to a particle from another particle
// static float force(float r, float alpha) {
//     float beta = 0.25f;
    
//     if (r < beta) 
//         return ((r / beta) - 1.0f) * 2.0f;
//     else if (beta <= r && r < 1.0f) 
//         return alpha * (1.0f - fabsf(2.0f * r - 1.0f - beta) / (1.0f - beta));
    
//     return 0.0f;
// }

// wrap : Check distance wrapped across screen and adjust deltas
// static void wrap(application_t *application, float *dx, float *dy) {
//     if (*dx >  application->width  / 2) *dx -= application->width;
//     if (*dx < -application->width  / 2) *dx += application->width;
//     if (*dy >  application->height / 2) *dy -= application->height;
//     if (*dy < -application->height / 2) *dy += application->height;
// }

// // magnitude : returns the magnitude of p1 in direction to p2
// static float magnitude(particle_t p1, particle_t p2, application_t *application) {
//     float dx = p2.position.x - p1.position.x;
//     float dy = p2.position.y - p1.position.y;

//     wrap(application, &dx, &dy);

//     return sqrtf(dx * dx + dy * dy);
// }

// // unit : returns a unit vector of the passed vector
// static vector2D_t unit(particle_t p1, particle_t p2, application_t *application) {
//     float dx = p2.position.x - p1.position.x;
//     float dy = p2.position.y - p1.position.y;

//     wrap(application, &dx, &dy);

//     float mag = sqrtf(dx * dx + dy * dy);

//     if (mag == 0.0f) return (vector2D_t) { .x = 0.0f, .y = 0.0f };

//     return (vector2D_t) {
//         .x = dx / mag,
//         .y = dy / mag
//     };
// }

// scale_vector : returns a scaled vector
// static vector2D_t scale_vector(float scalar, vector2D_t vector) {
//     return (vector2D_t) {
//         .x = scalar * vector.x,
//         .y = scalar * vector.y
//     };
// }

// // update_particles : updates the particles array with new positions based on attraction factor
// void update_particles(application_t *application, particle_t *particles) {
//     for (int i = 0; i < NUM_PARTICLES; i++) {
//         vector2D_t accelerations[NUM_PARTICLES] = { 0 };
//         vector2D_t *ap = accelerations;

//         // compute force on particle i from particle j
//         for (int j = 0; j < NUM_PARTICLES; j++) {
//             float r = 0.0f;

//             if (i == j) continue;
//             else if ((r = magnitude(particles[i], particles[j], application)) > MAXDISTANCE) 
//                 continue;
            
//             float f = force(r / MAXDISTANCE, alpha(particles[i].class, particles[j].class));
//             vector2D_t u = unit(particles[i], particles[j], application);

//             *ap++ = (vector2D_t) {
//                 .x = u.x * f,
//                 .y = u.y * f
//             };
//         }

//         // sum up the accelerations
//         vector2D_t a = { 0.0f, 0.0f };
//         for (int j = 0; j < NUM_PARTICLES; j++) {
//             if (accelerations[j].x == 0.0f && accelerations[j].y == 0.0f) continue;

//             a.x += accelerations[j].x;
//             a.y += accelerations[j].y;
//         }

//         float friction = powf(0.5f, DELTATIME / FRICTIONHALFLIFE);

//         update velocities
//         particles[i].velocity.x = scale_vector(friction, particles[i].velocity).x + a.x * DELTATIME;
//         particles[i].velocity.y = scale_vector(friction, particles[i].velocity).y + a.y * DELTATIME;

//         update positions
//         particles[i].position.x += particles[i].velocity.x * DELTATIME;
//         particles[i].position.y += particles[i].velocity.y * DELTATIME;

//         // handle screen wrapping
//         particles[i].position.x = fmodf(particles[i].position.x + application->width, application->width);
//         particles[i].position.y = fmodf(particles[i].position.y + application->height, application->height);
//     }
// }

