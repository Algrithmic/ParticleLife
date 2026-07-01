#version 430 core

flat in uint classData;

out vec4 FragColor;

uniform vec4 palette[8];

void main() {
    FragColor = palette[classData];
}