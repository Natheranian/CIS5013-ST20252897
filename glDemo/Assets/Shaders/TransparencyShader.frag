#version 410

varying highp vec3 colour;

layout (location=0) out vec4 fragColour;

void main() {
  fragColour = vec4(colour, 0.4);
}