#version 430
layout (location = 0) in vec3 position;

layout(location = 0) uniform mat4 lightViewProjectionMatrix;
layout(location = 1) uniform mat4 modelMatrix;

void main()
{
    gl_Position = lightViewProjectionMatrix * modelMatrix * vec4(position, 1.0f);
}