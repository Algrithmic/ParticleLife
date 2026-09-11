/**
 * @file border.vert
 * @brief Vertex shader for the toroidal world-boundary outline.
 *
 * Projects the four corners of the world rectangle (uploaded directly in
 * world-space) to clip space using the same projection as particle rendering.
 */
#version 430 core
layout (location = 0) in vec2 position;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(position, 0.0, 1.0);
}
