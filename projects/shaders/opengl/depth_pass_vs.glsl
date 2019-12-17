#version 460 core

layout (location = 0) in vec3 position;

layout(location = 0) uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
}