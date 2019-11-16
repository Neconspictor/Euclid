#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout(std140, binding = 0) buffer TransformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 transform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;

out vec4 positionWorld;

void main()
{
    gl_Position = transforms.transform * vec4(position, 1.0f);
    positionWorld = transforms.model * vec4(position, 1.0f);
    positionWorld -= vec4(transforms.view[3].rgb, 0.0);
    positionWorld.a = 0.0;
}