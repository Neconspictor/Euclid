#version 430 core

#define GROUP_NUM_X 16
#define GROUP_NUM_Y 8
#define REDUCE_BOUNDS_BLOCK_X 16
#define REDUCE_BOUNDS_BLOCK_Y 8


#define REDUCE_TILE_WIDTH REDUCE_BOUNDS_BLOCK_X * GROUP_NUM_X
#define REDUCE_TILE_HEIGHT REDUCE_BOUNDS_BLOCK_Y * GROUP_NUM_Y

#define REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE (GROUP_NUM_X*GROUP_NUM_Y)

layout (local_size_x = GROUP_NUM_X, local_size_y = GROUP_NUM_Y) in;
// An image to store data into.
//layout (rg32f, binding = 0) uniform image2D data;

layout (r32ui, binding = 0) uniform readonly uimage2D depthTexture;


layout(std430, binding = 2) buffer readonly BufferData
{
    vec4 mCameraNearFar;  // z and w component aren't used
    vec4 mColor;
} shader_data;

layout(std430, binding = 1) buffer writeonly BufferObject
{
    uint minResult; // a float value, but glsl doesn't support atomic operations on floats
    uint maxResult;
} writeOut;

shared uint groupMinValues[REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE];
shared uint groupMaxValues[REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE];


void main(void)
{
    uint minOfTile = 0x7f7fffff; //max float as unsigned int bits
    // for preserving ordering, it is necessary to use a positive number (are 0)                       
    uint maxOfTile = 0x0;   
   

    // NOTE: This variant is much slower (doesn't know why exactly...)
    /*const ivec2 globalID =  ivec2(gl_WorkGroupID.xy) * ivec2(REDUCE_TILE_WIDTH, REDUCE_TILE_HEIGHT);
    
    const ivec2 tileStart = globalID + ivec2(gl_LocalInvocationID.x * GROUP_NUM_X, gl_LocalInvocationID.y * GROUP_NUM_Y);
                          
    
    for (uint tileY = 0; tileY < REDUCE_BOUNDS_BLOCK_Y; ++tileY) {
        for (uint tileX = 0; tileX < REDUCE_BOUNDS_BLOCK_X; ++tileX) {
        
            ivec2 location = tileStart + ivec2(tileX, tileY);         
            uint depth = imageLoad(depthTexture, location).r;
            
            if (depth != 0) {
                minOfTile = min(minOfTile, depth);
                maxOfTile = max(maxOfTile, depth);
            }
        }
    }*/
    
    const ivec2 globalID =  ivec2(gl_WorkGroupID.xy) * ivec2(REDUCE_TILE_WIDTH, REDUCE_TILE_HEIGHT);
    
    const ivec2 tileStart = globalID + ivec2(gl_LocalInvocationID.xy);
                          
    
    for (uint tileY = 0; tileY < REDUCE_TILE_HEIGHT; tileY += REDUCE_BOUNDS_BLOCK_Y) {
        for (uint tileX = 0; tileX < REDUCE_TILE_WIDTH; tileX += REDUCE_BOUNDS_BLOCK_X) {
        
            ivec2 location = tileStart + ivec2(tileX, tileY);         
            uint depth = imageLoad(depthTexture, location).r;
            
            if (depth != 0) {
                minOfTile = min(minOfTile, depth);
                maxOfTile = max(maxOfTile, depth);
            }
        }
    }
    
    const uint groupIndex = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    
    groupMinValues[groupIndex] = minOfTile;
    groupMaxValues[groupIndex] = maxOfTile;
    
    groupMemoryBarrier();
    barrier();
    
    for (uint offset = REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE >> 1; offset > 0; offset >>= 1) {
        groupMinValues[groupIndex] = min(groupMinValues[groupIndex], groupMinValues[groupIndex + offset]);
        groupMaxValues[groupIndex] = max(groupMaxValues[groupIndex], groupMaxValues[groupIndex + offset]);
        groupMemoryBarrier();
        barrier();
    }
    
    if (groupIndex == 0) {
        atomicMin(writeOut.minResult,  groupMinValues[groupIndex]);
        atomicMax(writeOut.maxResult,  groupMaxValues[groupIndex]);
    }
    
    
    //atomicMin(writeOut.minResult, floatBitsToUint(0.1)); //floatBitsToUint(depth)
    //atomicMax(writeOut.maxResult, floatBitsToUint(1.0));
}

/*

// data that we can read or derived from the surface shader outputs
struct SurfaceData
{
    vec3 positionView;         // View space position
    //vec3 positionViewDX;       // Screen space derivatives
    //vec3 positionViewDY;       // of view space position
    //vec3 normal;               // View space normal
    //vec4 albedo;
    vec3 lightTexCoord;        // Texture coordinates and depth in light space, [0, 1]
    //vec3 lightTexCoordDX;      // Screen space partial derivatives
    //vec3 lightTexCoordDY;      // of light space texture coordinates.
};


SurfaceData ComputeSurfaceDataFromGBuffer(uint2 coord) {

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: This actually needs to be modified to ensure that it fits in local memory for any number of partitions!
// Needs to be based on max local memory size and max partition count.
// Currently set to 128 threads which works with up to 10 partitions (given 32KB local memory)
#define REDUCE_BOUNDS_BLOCK_X 16
#define REDUCE_BOUNDS_BLOCK_Y 8
#define REDUCE_BOUNDS_BLOCK_SIZE (REDUCE_BOUNDS_BLOCK_X*REDUCE_BOUNDS_BLOCK_Y)
#define PARTITIONS 1


// Store reduction results
RWStructuredBuffer<BoundsUint> gPartitionBoundsUint : register(u7);
StructuredBuffer<BoundsFloat> gPartitionBoundsReadOnly : register(t7);

// Store these as raw float arrays for reduction efficiency:
// Grouped by PARTITIONS
// Then grouped by SV_GroupIndex
#define REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE (PARTITIONS * REDUCE_BOUNDS_BLOCK_SIZE)
groupshared float3 sBoundsMin[REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE];
groupshared float3 sBoundsMax[REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE];


layout (local_size_x = PARTITIONS, local_size_y = 1) in;
void ClearPartitionBounds()
{
    gPartitionBoundsUint[gl_LocalInvocationIndex] = EmptyBoundsUint();
}


layout (local_size_x = REDUCE_BOUNDS_BLOCK_X, local_size_y = REDUCE_BOUNDS_BLOCK_Y) in;
void ReduceBoundsFromGBuffer(uint  groupIndex    : SV_GroupIndex)
{
    uint2 gbufferDim;
    gGBufferTextures[0].GetDimensions(gbufferDim.x, gbufferDim.y);

    // Initialize stack copy of partition data for this thread
    BoundsFloat boundsReduce[PARTITIONS];
    {
        for (uint partition = 0; partition < PARTITIONS; ++partition) {
            boundsReduce[partition] = EmptyBoundsFloat();
        }
    }
    
    // Loop over tile and reduce into local memory
    float nearZ = gPartitionsReadOnly[0].intervalBegin;
    float farZ = gPartitionsReadOnly[PARTITIONS - 1].intervalEnd;
    {
        uint2 tileStart = gl_WorkGroupID.xy * mReduceTileDim.xx + gl_LocalInvocationID.xy;
        for (uint tileY = 0; tileY < mReduceTileDim; tileY += REDUCE_BOUNDS_BLOCK_Y) {
            for (uint tileX = 0; tileX < mReduceTileDim; tileX += REDUCE_BOUNDS_BLOCK_X) {
                // Sample/compute surface data
                uint2 globalCoords = tileStart + uint2(tileX, tileY);
                SurfaceData data = ComputeSurfaceDataFromGBuffer(globalCoords);
                
                // Drop samples that fall outside the view frustum (clear color, etc)
                if (data.positionView.z >= nearZ && data.positionView.z < farZ) {
                    uint partition = 0;
                    for (uint i = 0; i < (PARTITIONS - 1); ++i) {
                        if (data.positionView.z >= gPartitionsReadOnly[i].intervalEnd) {
                            ++partition;
                        }
                    }

                    // Update relevant partition data for this thread
                    // This avoids the need for atomics since we're the only thread accessing this data
                    //Notice: After this steps, boundsReduce contains the min/max coords of the current tile
                    boundsReduce[partition].minCoord = min(boundsReduce[partition].minCoord, data.lightTexCoord.xyz);
                    boundsReduce[partition].maxCoord = max(boundsReduce[partition].maxCoord, data.lightTexCoord.xyz);
                }
            }
        }
    }
    
    
    // Copy result to shared memory for reduction
    // Notice: Shared memory is visible for all invocations in the local workgroup!
    // Thus, sBoundsMin / sBoundsMax are replicated for each global work group!
    {
        for (uint partition = 0; partition < PARTITIONS; ++partition) {
            uint index = (gl_LocalInvocationIndex * PARTITIONS + partition);
            sBoundsMin[index] = boundsReduce[partition].minCoord;
            sBoundsMax[index] = boundsReduce[partition].maxCoord;
        }
    }

    //GroupMemoryBarrierWithGroupSync();
    groupMemoryBarrier(); // esnures that all invocations see the updated sBoundsMin / sBoundsMax arrays
    
    // Now reduce our local memory data set to one element
    // After the end of this reduction, the first elements of sBoundsMin/sBoundsMax contain the values for each partition (partition 1 has index 0)
    //Notice: right shift(>>): halves the number
    for (uint offset = (REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE >> 1); offset >= PARTITIONS; offset >>= 1) {
        for (uint i = gl_LocalInvocationIndex; i < offset; i += REDUCE_BOUNDS_BLOCK_SIZE) {
            sBoundsMin[i] = min(sBoundsMin[i], sBoundsMin[offset + i]);
            sBoundsMax[i] = max(sBoundsMax[i], sBoundsMax[offset + i]);
        }
        //GroupMemoryBarrierWithGroupSync();
        groupMemoryBarrier();
    }

    // Now write out the result from this pass
    // Notice: the first 'PARTITIONS' local invocations, containing min/max of the current local work group 
    // write to the (global) final output. Using synchronization, at the end, the final min/max are than stored in gPartitionBoundsUint
    if (gl_LocalInvocationIndex < PARTITIONS) {
        InterlockedMin(gPartitionBoundsUint[gl_LocalInvocationIndex].minCoord.x, asuint(sBoundsMin[gl_LocalInvocationIndex].x));
        InterlockedMin(gPartitionBoundsUint[gl_LocalInvocationIndex].minCoord.y, asuint(sBoundsMin[gl_LocalInvocationIndex].y));
        InterlockedMin(gPartitionBoundsUint[gl_LocalInvocationIndex].minCoord.z, asuint(sBoundsMin[gl_LocalInvocationIndex].z));
        InterlockedMax(gPartitionBoundsUint[gl_LocalInvocationIndex].maxCoord.x, asuint(sBoundsMax[gl_LocalInvocationIndex].x));
        InterlockedMax(gPartitionBoundsUint[gl_LocalInvocationIndex].maxCoord.y, asuint(sBoundsMax[gl_LocalInvocationIndex].y));
        InterlockedMax(gPartitionBoundsUint[gl_LocalInvocationIndex].maxCoord.z, asuint(sBoundsMax[gl_LocalInvocationIndex].z));
    }
}*/