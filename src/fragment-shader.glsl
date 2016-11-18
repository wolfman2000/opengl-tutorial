#version 150

in vec3 Color;

out vec4 outColor;

void main() {
  outColor = vec4(1.0-Color.r, 1.0-Color.g, 1.0-Color.b, 1.0);
}

