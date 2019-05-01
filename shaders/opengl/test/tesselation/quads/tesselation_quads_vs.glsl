#version 450
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 texCoord;

out vec2 texCoord_tcs_in;


void main() {
  gl_Position = vertex;
  texCoord_tcs_in = texCoord;
}