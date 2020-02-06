#version 460 core
layout (location = 0) in vec3 position;

#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#include "interface/buffers.h"

out vec3 interpolatedVertexColor;
out vec3 positionLocal;

void main()
{ 
    const float reciprScaleOnscreen = 0.2; // change value to match resolution.    = (2 * ObjectSizeOnscreenInPixels / ScreenWidthInPixels)
    float w = (objectData.transform * vec4(0,0,0, 1)).w;
    w *= reciprScaleOnscreen;
    gl_Position = objectData.transform * vec4(position, 1.0);
    
    interpolatedVertexColor = vec3(1,0,0);
    positionLocal = position;
} 