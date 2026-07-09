/**
 * @file shaders.h
 * @brief Shader program state type and shader initialization API.
 *
 * Declares the shader_t handle bundle (graphics/compute programs and vertex
 * buffers) and the functions that build them from GLSL source files.
 */
#ifndef SHADERS_H
#define SHADERS_H

#include <stdint.h>

#define NUM_COORDINATES 9               ///< Vertices in the particle fan shape.
#define TOTAL_POINTS  NUM_COORDINATES * 2   ///< Floats in the vertex array (x, y per point).

typedef struct particle particle_t;

/// OpenGL handles for the simulation's shader programs and buffers.
typedef struct shader_data {
    uint32_t graphics_program;  ///< Program that draws particles.
    uint32_t compute_program;   ///< Program that steps the physics.
    uint32_t vao;               ///< Vertex array for instanced particle drawing.
    uint32_t vbo;               ///< Per-instance particle data buffer.
} shader_t;

uint8_t init_graphics(shader_t *shader_data, particle_t *particles, uint8_t count, ...);

uint8_t init_compute(shader_t *shader_data, char const *filename);

#endif