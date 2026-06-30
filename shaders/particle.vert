#version 430 core
layout (location = 0) in vec2 vertexData;
layout (location = 1) in vec2 positionOffset;
layout (location = 2) in uint particleClass;

flat out uint classData;

uniform mat4 projection;

void main() {
    vec2 worldPosition = vertexData + positionOffset;
    gl_Position = projection * vec4(worldPosition, 0.0, 1.0); 
    classData = particleClass;
}