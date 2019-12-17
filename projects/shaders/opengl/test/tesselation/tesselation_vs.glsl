#version 460 core
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 texCoord;

out vec4 position_ndc_tcs_in;
out vec2 texCoord_tcs_in;


void main() {
  position_ndc_tcs_in = vertex;
  texCoord_tcs_in = texCoord;
}