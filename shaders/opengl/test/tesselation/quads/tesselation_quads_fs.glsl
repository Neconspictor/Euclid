#version 450

//in vec2 texCoord_ndc_fs_in;

//in vec2 texCoord_tcs_in;

out vec4 color;
out vec4 luminance;
out vec2 motion;

void main() {
  color = vec4(10.0,10.0,10.0,1);
  luminance = vec4(0.0);
  motion = vec2(0.0);
}