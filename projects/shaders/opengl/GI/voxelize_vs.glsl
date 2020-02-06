#version 460 core

#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#include "interface/buffers.h"


#ifndef VOXEL_BASE_SIZE
#define  VOXEL_BASE_SIZE 128.0
#endif

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;

out VS_OUT {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
} vs_out;


void main()
{
    gl_Position = objectData.model * vec4(position, 1.0f); 
    vs_out.position = gl_Position.xyz;
    vs_out.normal = normalize(mat3(transpose(inverse(objectData.model))) * normal);
    vs_out.texCoords = texCoords;
}