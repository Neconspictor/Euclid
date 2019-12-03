#version 330 core

in vec2 texCoordsFS;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 luminance;

void main()
{ 
    color = vec4(1.0);
    luminance = vec4(0.0, 0.0, 0.0, 1.0);
}