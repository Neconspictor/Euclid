#version 430 core

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


#include "interface/cluster_interface.h"
#include "interface/light_interface.h"


layout (std140, binding = 0) uniform ConstantsUBO 
{
    Constants constants;
};

layout (std430, binding = 0) buffer clusterAABB 
{
    AABB clusters[];
};

/**
 * Contains the lights to cull.
 */
layout (std430, binding = 1) buffer lightSSBO 
{
    EnvironmentLight environmentLights[];
};


/**
 * Must have size of MAX_VISIBLES_LIGHTS * clusterCount
 */
layout (std430, binding = 2) buffer lightIndexSSBO 
{
    uint globalLightIndexList[];
};

/**
 * Stores offset and count into globalLightIndexList for each cluster.
 * => Size of array: amount of clusters
 */
layout (std430, binding = 3) buffer lightGridSSBO 
{
    LightGrid lightGrids[];
};

layout (std430, binding = 4) buffer globalIndexCountSSBO 
{
    uint globalIndexCount;
};

//Shared variables 
shared EnvironmentLight sharedLights[LOCAL_SIZE_X * LOCAL_SIZE_Y * LOCAL_SIZE_Z];

float sqDistPointAABB(vec3 point, uint clusterID);
bool testAABBWorld(uint light, uint clusterID);
bool testSphereAABB(uint light, uint clusterID);


void main()
{
    /*globalIndexCount = 0;
    uint threadCount = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
    uint lightCount  = environmentLights.length();
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
        sharedLights[gl_LocalInvocationIndex] = environmentLights[lightIndex];
        barrier();

        //Iterating within the current batch of lights
        for( uint light = 0; light < threadCount && (visibleLightCount < MAX_VISIBLES_LIGHTS); ++light){
            if( sharedLights[light].enabled  == 1){
            
                if (sharedLights[light].usesBoundingBox == 1) {
                    if (testAABBWorld(light, globalInvocationIndex)) {
                        visibleLightIndices[visibleLightCount] = batch * threadCount + light;
                        visibleLightCount += 1;
                    }
                
                } else {
                    if( testSphereAABB(light, globalInvocationIndex) ){
                        visibleLightIndices[visibleLightCount] = batch * threadCount + light;
                        visibleLightCount += 1;
                    }
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
    lightGrids[globalInvocationIndex].count = visibleLightCount;*/
}

float sqDistPointAABB(vec3 point, uint clusterID)
{
    float sqDist = 0.0;
    AABB currentCell = clusters[clusterID];
    //cluster[clusterID].maxPoint[3] = clusterID;
    for(int i = 0; i < 3; ++i){
        float v = point[i];
                
        if(v < currentCell.minView[i])
        {
            float dist = currentCell.minView[i] - v;
            sqDist += dist * dist;
        }
        if(v > currentCell.maxView[i])
        {
            float dist = v - currentCell.maxView[i];
            sqDist += dist * dist;
        }
    }

    return sqDist;
}

bool testAABBWorld(uint light, uint clusterID) {
    AABB currentCell = clusters[clusterID];
    EnvironmentLight envLight = environmentLights[light];
    
    return (currentCell.minWorld.x <= envLight.maxWorld.x && currentCell.maxWorld.x >= envLight.minWorld.x) &&
    (currentCell.minWorld.y <= envLight.maxWorld.y && currentCell.maxWorld.y >= envLight.minWorld.y) &&
    (currentCell.minWorld.z <= envLight.maxWorld.z && currentCell.maxWorld.z >= envLight.minWorld.z);
}

bool testSphereAABB(uint light, uint clusterID)
{
    float radius = sharedLights[light].sphereRange;
    vec3 center  = vec3(constants.view * sharedLights[light].position);
    float squaredDistance = sqDistPointAABB(center, clusterID);

    return squaredDistance <= (radius * radius);
}