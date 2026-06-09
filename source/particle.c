#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "SDL.h"

#include "../include/particle.h"
#include "../include/application.h"


// new_particle : creates a new particle
static particle_t new_particle(vector2D_t position, class_t class) {
    return (particle_t) {
        .position = position,
        .velocity = (vector2D_t) { .x = 0.0f, .y = 0.0f },
        .class = class
    };
}

// particle attraction matrix used for physics calculations
attraction_t attraction;

// init_particles : creates a pointer to an array of n particles
particle_t *init_particles(application_t *application, unsigned int n, unsigned int num_classes) {
    if (num_classes > MAXNUMCLASSES) {
        printf("particle.c : number of classes not supported\n");
        return NULL;
    }

    // initialize particles array
    particle_t *particles = (particle_t *) malloc(n * sizeof(particle_t));
    for (unsigned int i = 0; i < n; i++) {
        particles[i] = new_particle(
            (vector2D_t) { 
                SDL_rand(application->width), 
                SDL_rand(application->height) 
            },
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
static float alpha(class_t c1, class_t c2) {
    return attraction.matrix[c1 * attraction.nclass + c2];
}

// force : determines the force applied to a particle from another particle
static float force(float r, float alpha) {
    float beta = 0.25f;
    
    if (r < beta) 
        return ((r / beta) - 1.0f) * 2.0f;
    else if (beta <= r && r < 1.0f) 
        return alpha * (1.0f - fabsf(2.0f * r - 1.0f - beta) / (1.0f - beta));
    
    return 0.0f;
}

// wrap : Check distance wrapped across screen and adjust deltas
static void wrap(application_t *application, float *dx, float *dy) {
    if (*dx >  application->width  / 2) *dx -= application->width;
    if (*dx < -application->width  / 2) *dx += application->width;
    if (*dy >  application->height / 2) *dy -= application->height;
    if (*dy < -application->height / 2) *dy += application->height;
}

// magnitude : returns the magnitude of p1 in direction to p2
static float magnitude(particle_t p1, particle_t p2, application_t *application) {
    float dx = p2.position.x - p1.position.x;
    float dy = p2.position.y - p1.position.y;

    wrap(application, &dx, &dy);

    return sqrtf(dx * dx + dy * dy);
}

// unit : returns a unit vector of the passed vector
static vector2D_t unit(particle_t p1, particle_t p2, application_t *application) {
    float dx = p2.position.x - p1.position.x;
    float dy = p2.position.y - p1.position.y;

    wrap(application, &dx, &dy);

    float mag = sqrtf(dx * dx + dy * dy);

    if (mag == 0.0f) return (vector2D_t) { .x = 0.0f, .y = 0.0f };

    return (vector2D_t) {
        .x = dx / mag,
        .y = dy / mag
    };
}

// scale_vector : returns a scaled vector
static vector2D_t scale_vector(float scalar, vector2D_t vector) {
    return (vector2D_t) {
        .x = scalar * vector.x,
        .y = scalar * vector.y
    };
}

// update_particles : updates the particles array with new positions based on attraction factor
void update_particles(application_t *application, particle_t *particles) {
    for (int i = 0; i < NUMPARTICLES; i++) {
        vector2D_t accelerations[NUMPARTICLES] = { 0 };
        vector2D_t *ap = accelerations;

        // compute force on particle i from particle j
        for (int j = 0; j < NUMPARTICLES; j++) {
            float r = 0.0f;

            if (i == j) continue;
            else if ((r = magnitude(particles[i], particles[j], application)) > MAXDISTANCE) 
                continue;
            
            float f = force(r / MAXDISTANCE, alpha(particles[i].class, particles[j].class));
            vector2D_t u = unit(particles[i], particles[j], application);

            *ap++ = (vector2D_t) {
                .x = u.x * f,
                .y = u.y * f
            };
        }

        // sum up the accelerations
        vector2D_t a = { 0.0f, 0.0f };
        for (int j = 0; j < NUMPARTICLES; j++) {
            if (accelerations[j].x == 0.0f && accelerations[j].y == 0.0f) continue;

            a.x += accelerations[j].x;
            a.y += accelerations[j].y;
        }

        float friction = powf(0.5f, DELTATIME / FRICTIONHALFLIFE);

        // update velocities
        particles[i].velocity.x = scale_vector(friction, particles[i].velocity).x + a.x * DELTATIME;
        particles[i].velocity.y = scale_vector(friction, particles[i].velocity).y + a.y * DELTATIME;

        // update positions
        particles[i].position.x += particles[i].velocity.x * DELTATIME;
        particles[i].position.y += particles[i].velocity.y * DELTATIME;

        // handle screen wrapping
        particles[i].position.x = fmodf(particles[i].position.x + application->width, application->width);
        particles[i].position.y = fmodf(particles[i].position.y + application->height, application->height);
    }
}

// fill_particle : fills in the particle based on it's bounds
static void fill_particle(application_t *application, int start, int end, int particle_y, int y_offset) {
    for (int i = start; i < end + 1; i++) {
        // SDL_RenderPoint(application->renderer, i, particle_y + y_offset);
        // SDL_RenderPoint(application->renderer, i, particle_y - y_offset);
    }
}

// color classes classified by class_t
static SDL_Color rgba[MAXNUMCLASSES] = {
    { 255,  59,  48, 255 },
    {   0, 122, 255, 255 }, 
    {  52, 199,  89, 255 },
    { 255, 204,   0, 255 },
    { 175,  82, 222, 255 },
    { 255, 149,   0, 255 },
    { 50,  173, 230, 255 },
    { 255, 255, 255, 255 }
};

// draw_particle : draws a particle to the screen
void draw_particle(application_t *application, particle_t particle) {
    // set draw color of the particle
    // R, G, B, A
    // SDL_SetRenderDrawColor(application->renderer, rgba[particle.class].r, rgba[particle.class].g, rgba[particle.class].b, rgba[particle.class].a);

    // midpoint algorithm for drawing circles
    fill_particle(application, particle.position.x - RADIUS, particle.position.x + RADIUS, particle.position.y, 0);

    int x = RADIUS;
    int y = 0;
    int p = 1 - RADIUS;
    while (x > y) {
        y++;

        // Midpoint is inside or on the perimeter
        if (p <= 0) p = p + 2*y + 1;
        else {  // midpoint is outside the perimeter
            x--;
            p = p + 2*y - 2*x + 1;
        }

        // all points found 
        if (x < y) break;

        fill_particle(application, particle.position.x - x, particle.position.x + x, particle.position.y, y);

        if (x != y)
            fill_particle(application, particle.position.x - y, particle.position.x + y, particle.position.y, x);
    }
}

