#version 330 core

in vec2 texCoordsFS;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 luminance;

const float offset = 1.0f / 300.0f;

void main()
{ 
    color = vec4(1.0);
}