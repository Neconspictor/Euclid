#version 460 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 transform;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = aPos;
}