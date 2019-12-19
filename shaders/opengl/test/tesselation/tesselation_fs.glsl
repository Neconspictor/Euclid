#version 460 core

in vec4 position_ndc_fs_in;
in vec2 texCoord_ndc_fs_in;

out vec4 color;
out vec4 luminance;

void main() {
  color = vec4(10.0,10.0,10.0,1);
  luminance = vec4(0.0);
}