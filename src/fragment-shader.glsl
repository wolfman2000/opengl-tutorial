#version 150

in float GrayScale;

out vec4 outColor;

void main() {
  outColor = vec4(GrayScale, GrayScale, GrayScale, 1.0);
}

