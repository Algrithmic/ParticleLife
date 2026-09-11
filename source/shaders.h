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
    uint32_t graphics;          ///< Program that draws particles.
    uint32_t compute;           ///< Program that steps the physics.
    uint32_t vao;               ///< Vertex array for instanced particle drawing.
    uint32_t vbo;               ///< Per-instance particle data buffer.
    uint32_t particle_ssbo;     ///< Shared Buffer Object for particle array.
    uint32_t attraction_ssbo;   ///< Shared Buffer Object for attraction matrix.
    uint32_t border_program;    ///< Program that draws the toroidal world-boundary outline.
    uint32_t border_vao;        ///< Vertex array for the boundary outline (4 corners).
    uint32_t border_vbo;        ///< Vertex buffer holding the 4 world-space corner points.
} shader_t;

/// Compile and link the graphics program from a variadic list of .vert/.frag files.
bool init_graphics(shader_t *shaders, world_t *world, uint8_t count, ...);

/// Compile and link the compute program from a single .comp source file.
bool init_compute(shader_t *shaders, char const *filename);

/// Compile, link, and set up buffers for the world-boundary outline program.
bool init_border(shader_t *shaders);

/// Re-upload the 4 corner points of the world-boundary outline rectangle.
void update_border_vbo(shader_t *shaders, float world_width, float world_height);

/// Draw the world-boundary outline as a white rectangle using the given projection.
void draw_border(shader_t *shaders, float const projection[16]);

/// Create and bind the particle and attraction shader storage buffers (SSBOs).
bool init_buffers(shader_t *shaders, world_t *world);

/// Re-upload a contiguous range of particles to the particle SSBO.
void update_particle_ssbo(shader_t *shaders, world_t *world, uint32_t offset, uint32_t count);

/// Re-upload the full attraction matrix to its SSBO.
void update_attraction_ssbo(shader_t *shaders, world_t *world);

#endif