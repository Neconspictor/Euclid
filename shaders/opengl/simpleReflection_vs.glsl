#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 Position;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 normalMatrix;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
    Normal = mat3(normalMatrix) * normal;
    Position = vec3(model * vec4(position, 1.0f));
}  