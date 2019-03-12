#version 430

#include "shadow/cascade_common.glsl"

layout (location = 0) in vec3 position;

//layout(location = 0) uniform mat4 lightViewProjectionMatrix;
layout(location = 0) uniform uint cascadeIdx;
layout(location = 1) uniform mat4 modelMatrix;

layout(binding = 0) buffer Cascades {
    CascadeData data;
} cascades;

void main()
{
    gl_Position = cascades.data.lightViewProjectionMatrices[cascadeIdx] * modelMatrix * vec4(position, 1.0f);
}