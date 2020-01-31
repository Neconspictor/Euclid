#version 460 core
layout(location = 0) in vec4 vertex;

void main() {
  gl_Position = vertex;
  gl_Position.z = 1.0;
}