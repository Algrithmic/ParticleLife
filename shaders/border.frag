/**
 * @file border.frag
 * @brief Fragment shader for the toroidal world-boundary outline.
 *
 * Draws every fragment solid white; the outline is a thin visual marker, not
 * a class-colored element.
 */
#version 430 core
out vec4 fragColor;

void main() {
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
