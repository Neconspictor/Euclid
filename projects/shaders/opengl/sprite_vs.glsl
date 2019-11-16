#version 330 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 transform;

out vec2 texCoordsFS;

void main()
{
    gl_Position = transform * position; 
    texCoordsFS = texCoords;
} 