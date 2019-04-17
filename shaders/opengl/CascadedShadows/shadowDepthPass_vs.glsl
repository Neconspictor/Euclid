#version 430

#include "shadow/cascade_common.glsl"

layout (location = 0) in vec3 position;

//layout(location = 0) uniform mat4 lightViewProjectionMatrix;
layout(location = 0) uniform uint cascadeIdx;

layout(binding = 0) buffer TransformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 transform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;

layout(binding = 1) buffer Cascades {
    CascadeData data;
} cascades;

void main()
{
    gl_Position = cascades.data.lightViewProjectionMatrices[cascadeIdx] * transforms.model * vec4(position, 1.0f);
}