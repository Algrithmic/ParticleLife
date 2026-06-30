#ifndef SHADERS_H
#define SHADERS_H

#include <stdint.h>

#define NUM_COORDINATES 9
#define TOTAL_POINTS  NUM_COORDINATES * 2

typedef struct particle particle_t;

typedef struct shader_data {
    uint32_t graphics_program;
    uint32_t compute_program;
    uint32_t vao;
    uint32_t vbo;
} shader_t;

uint8_t init_graphics(shader_t *shader_data, particle_t *particles, uint8_t count, ...);

uint8_t init_compute(shader_t *shader_data, char const *filename);

#endif