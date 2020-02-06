#version 460 core

#ifndef BONE_ANIMATION
#define BONE_ANIMATION 0
#endif


#if BONE_ANIMATION

#ifndef BONE_ANIMATION_TRAFOS_BINDING_POINT
#define BONE_ANIMATION_TRAFOS_BINDING_POINT 1
#endif

#endif

layout (location = 0) in vec3  position;
layout (location = 1) in vec3  normal;
layout (location = 2) in vec2  texCoords;
layout (location = 3) in vec3  tangent;
#if BONE_ANIMATION
layout (location = 4) in uvec4 boneId;
layout (location = 5) in vec4  boneWeight;
#endif

#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#include "interface/buffers.h"


#if BONE_ANIMATION
layout(column_major, std140, binding = BONE_ANIMATION_TRAFOS_BINDING_POINT) buffer BoneAnimationBuffer {
    mat4[] trafos;
} boneTrafos;

#endif


void main()
{

#if BONE_ANIMATION
    mat4 boneTrafo = boneTrafos.trafos[boneId[0]] * boneWeight[0];
    boneTrafo += boneTrafos.trafos[boneId[1]] * boneWeight[1];
    boneTrafo += boneTrafos.trafos[boneId[2]] * boneWeight[2];
    boneTrafo += boneTrafos.trafos[boneId[3]] * boneWeight[3];
    
    vec4 positionLocal = boneTrafo * vec4(position, 1.0f);
#else 
    vec4 positionLocal = vec4(position, 1.0f);
#endif

    gl_Position = objectData.transform * positionLocal;
}