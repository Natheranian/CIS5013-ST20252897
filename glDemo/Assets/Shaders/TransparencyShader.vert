#version 410

uniform mat4 mvpMatrix;

layout (location=0) in vec3 vertexPos;

void main(void) {

    gl_Position = mvpMatrix * vec4(vertexPos, 1.0);
}