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