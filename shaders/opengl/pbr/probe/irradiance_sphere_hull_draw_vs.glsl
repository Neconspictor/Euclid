#version 430 core
layout (location = 0) in vec3 position;

layout(binding = 0) buffer TransformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 transform;
    mat4 prevTransform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;

out vec3 positionWS;
out vec3 positionVS;
out mat4 viewMatrix;
out vec3 positionLS;

void main()
{ 
    
    positionLS = position;
    const vec4 pos = vec4(position, 1.0);
    gl_Position = transforms.transform * pos;
    positionWS = (transforms.model * pos).xyz;
    positionVS = (transforms.modelView * pos).xyz;
    viewMatrix = transforms.view;
    
} 