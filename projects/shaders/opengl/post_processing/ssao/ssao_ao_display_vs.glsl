#version 460 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texCoords;


out vec2 texCoordsFS;

void main()
{
    gl_Position = position;
    texCoordsFS = texCoords;
}