#version 460 core
layout (location = 0) in vec3 position;

#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#include "interface/buffers.h"

void main()
{ 
    gl_Position = objectData.transform * vec4(position, 1.0f);
} 