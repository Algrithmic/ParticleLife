#ifndef SHADERS_H
#define SHADERS_H

#include <stdint.h>

#define NUM_COORDINATES 9
#define TOTAL_VERTICES  NUM_COORDINATES * 2

typedef struct particle particle_t;

typedef struct shader_data {
    uint32_t program;
    uint32_t vao;
    uint32_t vbo;
    uint8_t  ok;
} shader_t;

shader_t init_shaders(particle_t *particles, uint8_t count, ...);

#endif