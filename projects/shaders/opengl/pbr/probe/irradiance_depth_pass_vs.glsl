#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;


#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#include "interface/buffers.h"

out vec4 positionWorld;

void main()
{
    gl_Position = objectData.transform * vec4(position, 1.0f);
    positionWorld = objectData.model * vec4(position, 1.0f);
    positionWorld -= vec4(constants.viewGPass[3].rgb, 0.0);
    positionWorld.a = 0.0;
}