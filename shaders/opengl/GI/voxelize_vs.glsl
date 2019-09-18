#version 430 core
#ifndef PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT
#define PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT 0
#endif

#ifndef VOXEL_BASE_SIZE
#define  VOXEL_BASE_SIZE 128.0
#endif

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout(binding = PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT) buffer TransformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 transform;
    mat4 prevTransform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;

out VS_OUT {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
} vs_out;


void main()
{
    gl_Position = transforms.model * vec4(position, 1.0f); 
    vs_out.position = gl_Position.xyz;
    vs_out.normal = normalize(mat3(transpose(inverse(transforms.model))) * normal);
    vs_out.texCoords = texCoords;
}