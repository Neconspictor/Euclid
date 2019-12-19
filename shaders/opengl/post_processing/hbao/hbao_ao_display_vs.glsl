#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 transform;

out vec2 texCoordsFS;

void main()
{
    gl_Position = transform * vec4(position.xy, 0.0f, 1.0f);
    texCoordsFS = texCoords;
}