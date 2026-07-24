#version 430 core
#define MAX_NUM_CLASSES 8

flat in uint classData;

out vec4 FragColor;

uniform vec4 palette[MAX_NUM_CLASSES];

void main() {
    FragColor = palette[classData];
}