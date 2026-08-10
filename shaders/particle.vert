/**
 * @file particle.vert
 * @brief Vertex shader for instanced particle rendering.
 *
 * Draws each particle as a small fan-shaped disc: the shared shape vertices
 * (location 0) are scaled by radius and offset by the per-instance particle
 * position (location 1), then projected to clip space. Passes the particle's
 * class (gl_InstanceID % nclass) to the fragment shader for coloring.
 */
#version 430 core
layout (location = 0) in vec2 vertexData;
layout (location = 1) in vec2 positionOffset;

flat out uint classData;

uniform float radius;
uniform uint  nclass;
uniform mat4  projection;

void main() {
    vec2 worldPosition = (radius * vertexData) + positionOffset;
    gl_Position = projection * vec4(worldPosition, 0.0, 1.0); 
    classData = gl_InstanceID % nclass;
}