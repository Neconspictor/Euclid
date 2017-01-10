#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex;

uniform mat4 transform;
uniform mat4 modelView;
uniform mat4 normalMatrix;

out vec3 normalVS;
out vec3 fragmentPosition;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
    normalVS = vec3(normalMatrix * vec4(normal, 1.0f));
    fragmentPosition = vec3(modelView * vec4(position, 1.0f));
} 