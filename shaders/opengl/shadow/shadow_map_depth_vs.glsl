#version 430

layout (location = 0) in vec3 position;

layout(std140, binding = 0) buffer TransformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 transform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;

uniform mat4 lightViewProjection;

void main()
{
    gl_Position = lightViewProjection * transforms.model * vec4(position, 1.0f);
}