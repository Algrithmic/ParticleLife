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

typedef struct application application_t;
typedef struct particle particle_t;

/// OpenGL handles for the simulation's shader programs and buffers.
typedef struct shader_handles {
    uint32_t graphics_program;  ///< Program that draws particles.
    uint32_t compute_program;   ///< Program that steps the physics.
    uint32_t vao;               ///< Vertex array for instanced particle drawing.
    uint32_t vbo;               ///< Per-instance particle data buffer.
    uint32_t particle_ssbo;     ///< Shared Buffer Object for particle array.
    uint32_t attraction_ssbo;   ///< Shared Buffer Object for attraction matrix.
} shader_t;

/// Compile and link the graphics program from a variadic list of .vert/.frag files.
bool init_graphics(application_t *application, uint8_t count, ...);

/// Compile and link the compute program from a single .comp source file.
bool init_compute(shader_t *shader_data, char const *filename);

/// Create and bind the particle and attraction shader storage buffers (SSBOs).
bool init_buffers(application_t *application);

/// Re-upload a contiguous range of particles to the particle SSBO.
void update_particle_ssbo(application_t *application, uint32_t offset, uint32_t count);

/// Re-upload the full attraction matrix to its SSBO.
void update_attraction_ssbo(application_t *application);

#endif