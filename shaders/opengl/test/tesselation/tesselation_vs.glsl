#version 450
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 texCoord;

out VS_OUT {
    vec2 texCoord;
} vs_out;

void main() {
  gl_Position = vertex;
  vs_out.texCoord = texCoord;
}