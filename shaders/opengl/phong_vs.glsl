#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 transform;
uniform mat4 model;

out vec3 normalVS;
out vec3 fragmentPosition;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
    normalVS = mat3(transpose(inverse(model))) * normal;
    fragmentPosition = vec3(model * vec4(position, 1.0f));
} 