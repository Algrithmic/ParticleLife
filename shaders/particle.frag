/**
 * @file particle.frag
 * @brief Fragment shader that colors each particle by its class.
 *
 * Looks up the flat class index passed from the vertex shader in the color
 * palette uniform and outputs that RGBA color for every fragment of the particle.
 */
#version 430 core
#define MAX_NUM_CLASSES 8

flat in uint classData;

out vec4 FragColor;

uniform vec4 palette[MAX_NUM_CLASSES];

void main() {
    FragColor = palette[classData];
}