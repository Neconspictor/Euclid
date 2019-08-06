#version 430 core

#define MAX_VISIBLES_LIGHTS 100
#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 9
#define LOCAL_SIZE_Z 4

layout(local_size_x = LOCAL_SIZE_X, local_size_y = LOCAL_SIZE_Y, local_size_z = LOCAL_SIZE_Z) in;


struct PointLight
{
    vec4 position;
    vec4 color;
    uint enabled;
    float intensity;
    float range;
};

struct LightGrid
{
    uint offset;
    uint count;
};

struct AABB
{
    vec4 minPoint;
    vec4 maxPoint;
};

layout (std140, binding = 0) uniform ConstantsUBO 
{
    mat4 viewMatrix;
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
    PointLight pointLights[];
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
shared PointLight sharedLights[LOCAL_SIZE_X * LOCAL_SIZE_Y * LOCAL_SIZE_Z];

float sqDistPointAABB(vec3 point, uint clusterID);
bool testSphereAABB(uint light, uint clusterID);


void main()
{
    globalIndexCount = 0;
    uint threadCount = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
    uint lightCount  = pointLights.length();
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
        sharedLights[gl_LocalInvocationIndex] = pointLights[lightIndex];
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

float sqDistPointAABB(vec3 point, uint clusterID)
{
    float sqDist = 0.0;
    AABB currentCell = clusters[clusterID];
    //cluster[clusterID].maxPoint[3] = clusterID;
    for(int i = 0; i < 3; ++i){
        float v = point[i];
                
        if(v < currentCell.minPoint[i])
        {
            float dist = currentCell.minPoint[i] - v;
            sqDist += dist * dist;
        }
        if(v > currentCell.maxPoint[i])
        {
            float dist = v - currentCell.maxPoint[i];
            sqDist += dist * dist;
        }
    }

    return sqDist;
}

bool testSphereAABB(uint light, uint clusterID)
{
    float radius = sharedLights[light].range;
    vec3 center  = vec3(viewMatrix * sharedLights[light].position);
    float squaredDistance = sqDistPointAABB(center, clusterID);

    return squaredDistance <= (radius * radius);
}