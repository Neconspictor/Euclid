#version 460 core
layout (location = 0) in vec3 position;

layout(std140, binding = 0) buffer TransformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 transform;
    mat4 prevTransform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;

void main()
{ 
    gl_Position = transforms.transform * vec4(position, 1.0f);
} 