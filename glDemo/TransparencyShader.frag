#version 410

varying highp vec3 color;

layout (location=0) out vec4 fragColour;

void main() {
  fragColour = vec4(color, 0.5);
}