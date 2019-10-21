/**
 * Calculates the minimum and maximum z-value of the scene (seen from the camera)  
 */
#version 430 core

#define GROUP_NUM_X 16
#define GROUP_NUM_Y 8
#define BLOCK_X 16
#define BLOCK_Y 8


#define TILE_WIDTH BLOCK_X * GROUP_NUM_X
#define TILE_HEIGHT BLOCK_Y * GROUP_NUM_Y

#define SHARED_MEMORY_ARRAY_SIZE (GROUP_NUM_X*GROUP_NUM_Y)

layout (local_size_x = GROUP_NUM_X, local_size_y = GROUP_NUM_Y) in;
// An image to store data into.
//layout (rg32f, binding = 0) uniform image2D data;

layout (binding = 0) uniform sampler2D depthMap;
layout (binding = 1) uniform usampler2D stencilMap;

layout(r32ui, binding = 0) image1D waterMinDepths;

shared float groupMinValues[SHARED_MEMORY_ARRAY_SIZE];


void main(void)
{
    float localMin = 1.0;
    
    const ivec2 globalID =  ivec2(gl_WorkGroupID.xy) * ivec2(TILE_WIDTH, TILE_HEIGHT);
    const ivec2 tileStart = globalID + ivec2(gl_LocalInvocationID.xy);

    
    for (uint tileY = 0; tileY < TILE_HEIGHT; tileY += BLOCK_Y) {
        for (uint tileX = 0; tileX < TILE_WIDTH; tileX += BLOCK_X) {
        
            ivec2 positionScreen = tileStart + ivec2(tileX, tileY);
            const float depth = texelFetch(depthMap, positionScreen, 0).r;
            const uint stencil = texelFetch(stencilMap, positionScreen, 0).r;
            
            if (stencil != 0) {
                localMin = min(localMin, depth);
            }
        }
    }
    
    const uint groupIndex = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    
    groupMinValues[groupIndex] = localMin;
    
    groupMemoryBarrier();
    barrier();
    
    for (uint offset = (SHARED_MEMORY_ARRAY_SIZE >> 1); offset >= 1; offset >>= 1) {
        
        for (uint i = groupIndex; i < offset; i += (GROUP_NUM_X*GROUP_NUM_Y)) {
            groupMinValues[i] = min(groupMinValues[i], groupMinValues[i + offset]);
        }
        
        groupMemoryBarrier();
        barrier();
    }
    
    if (groupIndex == 0) {
        // Using atomic min/max is much faster than using a spin lock
        // This approach assumes that the values are >= 0 ; otherwise correct number order isn't guaranteed!
        float normalizedDepth = 0.5 * groupMinValues[groupIndex] + 0.5;
        imageAtomicMin(waterMinDepths, gl_LocalInvocationID.y, floatBitsToUint(normalizedDepth));
    }
}