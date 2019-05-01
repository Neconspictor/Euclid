#version 450
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 texCoord;

out vec2 texCoord_tcs_in;

uniform mat4 transform;


void main() {
  // use object space as tesselation evaluation shader projects the 
  // position into clip space!
  gl_Position = transform * vec4(vertex.xyz, 1.0);
  texCoord_tcs_in = texCoord;
}