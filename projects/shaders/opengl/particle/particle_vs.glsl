#version 330 core
layout (location = 0) in vec2 position;

uniform mat4 transform;

out vec2 texCoordsFS;

void main()
{
    gl_Position = transform * vec4(position, 0.0, 1.0); 
    texCoordsFS = 2.0 * position;
} 