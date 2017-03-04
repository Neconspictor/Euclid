#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 transform;
uniform mat4 model;

out vec3 fragPos;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
    fragPos = vec3(model * vec4(position, 1.0));
}