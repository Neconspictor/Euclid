#version 460 core

in vec2 texCoordsFS;
out vec4 color;

uniform sampler2D sprite;

void main()
{ 
    color = vec4(vec3(texture(sprite, texCoordsFS).r), 1.0);
}