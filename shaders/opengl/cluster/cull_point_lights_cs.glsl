#version 460 core

#ifndef MAX_VISIBLES_LIGHTS
    #define MAX_VISIBLES_LIGHTS 100
#endif
#ifndef LOCAL_SIZE_X
    #define LOCAL_SIZE_X 16
#endif
#ifndef LOCAL_SIZE_Y
    #define LOCAL_SIZE_Y 8
#endif
#ifndef LOCAL_SIZE_Z
    #define LOCAL_SIZE_Z 4
#endif

layout(local_size_x = LOCAL_SIZE_X, local_size_y = LOCAL_SIZE_Y, local_size_z = LOCAL_SIZE_Z) in;

#include "interface/light_interface.h"

/**
 * Contains the lights to cull.
 */
layout (std430, binding = 4) buffer lightSSBO 
{
    PointLight lights[];
};

//Shared variables 
//shared PointLight sharedLights[LOCAL_SIZE_X * LOCAL_SIZE_Y * LOCAL_SIZE_Z];

#define LIGHT_CLASS PointLight
#define SUPPORTS_SPHERE_RANGE 1
#include "cluster/cull_lights_common.glsl"


void main()
{
    globalIndexCount = 0;
    uint threadCount = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
    uint lightCount  = lights.length();
    uint numBatches = (lightCount + threadCount -1) / threadCount;

    uvec3 globalSize = gl_NumWorkGroups * gl_WorkGroupSize;
    uint globalInvocationIndex = gl_GlobalInvocationID.z * globalSize.x * globalSize.y + 
                                 gl_GlobalInvocationID.y * globalSize.x + 
                                 gl_GlobalInvocationID.x;
    
    uint visibleLightCount = 0;
    uint visibleLightIndices[MAX_VISIBLES_LIGHTS];

    for( uint batch = 0; batch < numBatches; ++batch){
        uint lightIndex = batch * threadCount + gl_LocalInvocationIndex;

        //Prevent overflow by clamping to last light which is always null
        lightIndex = min(lightIndex, lightCount);

        //Populating shared light array
        sharedLights[gl_LocalInvocationIndex] = lights[lightIndex];
        barrier();

        //Iterating within the current batch of lights
        for( uint light = 0; light < threadCount && (visibleLightCount < MAX_VISIBLES_LIGHTS); ++light){
            if( sharedLights[light].enabled  == 1){
                if( testSphereAABB(light, globalInvocationIndex) ){
                    visibleLightIndices[visibleLightCount] = batch * threadCount + light;
                    visibleLightCount += 1;
                }
            }
        }
    }

    //We want all thread groups to have completed the light tests before continuing
    barrier();

    uint offset = atomicAdd(globalIndexCount, visibleLightCount);

    for(uint i = 0; i < visibleLightCount; ++i){
        globalLightIndexList[offset + i] = visibleLightIndices[i];
    }

    lightGrids[globalInvocationIndex].offset = offset;
    lightGrids[globalInvocationIndex].count = visibleLightCount;
}