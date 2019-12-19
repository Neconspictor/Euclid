#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec2 texCoord_tcs_in;

uniform mat4 transform;

void main() {
  // use object space as tesselation evaluation shader projects the 
  // position into clip space!
  gl_Position = transform * vec4(position, 1.0);
  
  /*if (position.xyz == vec3(0.0)) {
    gl_Position = transform * vec4(vec3(0.0, 1.0, 0.0), 1.0);
  }
  
  if (position.xyz == vec3(1.0, 0.0, 0.0)) {
    gl_Position = transform * vec4(vec3(1.0, 1.0, 0.0), 1.0);
  }*/
  
  texCoord_tcs_in = texCoords;
}