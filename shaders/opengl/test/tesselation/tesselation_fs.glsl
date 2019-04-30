#version 450
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec2 texCoord;

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 color;
out vec4 luminance;

void main() {
  color = vec4(10.0,10.0,10.0,1);
  luminance = vec4(0.0);
}