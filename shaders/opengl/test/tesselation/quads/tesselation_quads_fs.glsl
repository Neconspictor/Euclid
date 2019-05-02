#version 450

//in vec2 texCoord_ndc_fs_in;

//in vec2 texCoord_tcs_in;

layout(location = 0)out vec4 color;
layout(location = 1)out vec4 luminance;
layout(location = 2)out vec2 motion;
layout(location = 3)out float depth;

void main() {
  color = vec4(10.0,10.0,10.0,1);
  luminance = vec4(0.0);
  motion = vec2(0.0);
  depth = gl_FragCoord.z;
}