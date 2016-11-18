#version 150

in vec2 position;
in float grayscale;

out float GrayScale;

void main() {
  GrayScale = grayscale;
  gl_Position = vec4(position, 0.0, 1.0);
}

