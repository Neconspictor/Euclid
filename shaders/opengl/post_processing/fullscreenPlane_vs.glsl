#version 330
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 tex;

out vec2 texCoord;


void main() {
  gl_Position = vertex;
  texCoord = tex;
}