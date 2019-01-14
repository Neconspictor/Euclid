#version 430 core

#include "test/compute/SDSM.glsl"

// The number of partitions to produce
//#ifndef PARTITIONS
//#define PARTITIONS 4
//#endif 

#define GROUP_NUM_X 16
#define GROUP_NUM_Y 8
#define REDUCE_BOUNDS_BLOCK_X 16
#define REDUCE_BOUNDS_BLOCK_Y 8


#define REDUCE_TILE_WIDTH REDUCE_BOUNDS_BLOCK_X * GROUP_NUM_X
#define REDUCE_TILE_HEIGHT REDUCE_BOUNDS_BLOCK_Y * GROUP_NUM_Y

#define REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE (PARTITIONS * GROUP_NUM_X*GROUP_NUM_Y)

layout (local_size_x = GROUP_NUM_X, local_size_y = GROUP_NUM_Y) in;
// An image to store data into.
//layout (rg32f, binding = 0) uniform image2D data;

layout (r32ui, binding = 0) uniform readonly uimage2D depthTexture;


layout(std430, binding = 2) buffer readonly BufferData
{
    vec4 mCameraNearFar;  // z and w component aren't used
    vec4 mColor;
    mat4 mCameraProj;
    mat4 mCameraViewToLightProj;
    Partition partitions[PARTITIONS];
} shader_data;

layout(std430, binding = 1) buffer BufferObject
{
    BoundsUint results[PARTITIONS];
    uint lock;
    vec3 _pad0;
    //uvec3 minResults[PARTITIONS]; // a float value, but glsl doesn't support atomic operations on floats
    //uvec3 maxResults[PARTITIONS];
    
} writeOut;

shared vec3 groupMinValues[REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE];
shared vec3 groupMaxValues[REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE];

void takeMinMaxLock() {
    uint lockAvailable;
    
    do {
        //memoryBarrier();
        lockAvailable = atomicCompSwap(writeOut.lock, 0, 1);
    } while(lockAvailable == 1);
};

void releaseMinMaxLock() {
    writeOut.lock = 0;
    //atomicExchange(writeOut.lock, 0);
};

/**
 * NOTE: Function assumes shader_data.mCameraViewToLightProj to be orthographic
 * Result is light space - clip space mapped to [0,1] (not [-1,1]!)
 */
vec3 projectIntoLightTexCoord(in vec3 positionView) {
    vec4 positionLight = shader_data.mCameraViewToLightProj * vec4(positionView, 1.0f);
    
    // perspective division and mapping from [-1,1] -> [0,1]
    vec3 texCoord = (positionLight.xyz / positionLight.w)* 0.5 + vec3(0.5);

    return texCoord;
}

vec3 computePositionViewFromZ(in vec2 positionNDC, float viewSpaceZ) {
    // For more information: https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy/46118945#46118945
    
    vec2 screenSpaceRay = vec2(
        viewSpaceZ * positionNDC.x / shader_data.mCameraProj[0][0],
        viewSpaceZ * positionNDC.y / shader_data.mCameraProj[1][1]);
                               
    vec3 positionView;
    positionView.z = -viewSpaceZ;
    positionView.xy = screenSpaceRay.xy;
    
    
    
    return positionView;
}


SurfaceData computeSurfaceData(ivec2 positionScreen) {
    
    SurfaceData data;
    data.depth = uintBitsToFloat(imageLoad(depthTexture, positionScreen).r);
    float depth = data.depth;
      
      
    vec2 gBufferDim = vec2(imageSize(depthTexture));
      
    //In OpenGl the mapping works a bit differently: (0,0) should be mapped to (-1, -1)
    // as the origin in NDC is in OpenGL (-1, -1) and not (-1, 1) like in DirectX11
    // Further in OpenGL is in screen space the origin (0,0) at the lower left corner
    // whereas in DirectX it is the top-left corner. So, pixel coordinates in OpenGL are handled differently
    // than in DirectX. The following mapping thus does basically the same although the result is differently.
    // (The result will be flipped on the y-axis, screen space (1,1) in DirectX will map to (1,-1) in OpenGL)
    // See also: http://www.songho.ca/opengl/gl_transform.html
    vec2 screenPixelOffset = vec2(2.0f, 2.0f) / gBufferDim;
    
    // Note: add 0.5f to the positionScreen for minimzing rounding errors
    vec2 positionNDC = (positionScreen + 0.5f) * 2.0f / gBufferDim + vec2(-1.0f, -1.0f);
    
    // Unproject depth z value into view space
    float z_ndc = 2.0 * depth - 1.0;
    float viewSpaceZ =  shader_data.mCameraProj[3][2] / (z_ndc + shader_data.mCameraProj[2][2]); //TODO       

    data.positionView = computePositionViewFromZ(positionNDC, viewSpaceZ);
      
    // Solve for light space position and screen-space derivatives
    data.lightTexCoord = projectIntoLightTexCoord(data.positionView);
    //data.lightTexCoord = vec3(positionNDC, viewSpaceZ);
    data.lightTexCoord = data.positionView;
    
    return data;
}


void main(void)
{
    BoundsFloat bounds[PARTITIONS]; 
    for (uint i = 0; i < PARTITIONS; ++i) {
        bounds[i] = EmptyBoundsFloat();
    }
    
       
    const ivec2 globalID =  ivec2(gl_WorkGroupID.xy) * ivec2(REDUCE_TILE_WIDTH, REDUCE_TILE_HEIGHT);
    
    const ivec2 tileStart = globalID + ivec2(gl_LocalInvocationID.xy);
    
    const float nearZ = shader_data.partitions[0].intervalBegin;
    const float farZ = shader_data.partitions[PARTITIONS-1].intervalEnd;
                          
    
    for (uint tileY = 0; tileY < REDUCE_TILE_HEIGHT; tileY += REDUCE_BOUNDS_BLOCK_Y) {
        for (uint tileX = 0; tileX < REDUCE_TILE_WIDTH; tileX += REDUCE_BOUNDS_BLOCK_X) {
        
            ivec2 positionScreen = tileStart + ivec2(tileX, tileY);         
            SurfaceData data = computeSurfaceData(positionScreen);
            
            /*if (data.depth != 0) {
            
                //for (uint i = 0; i < PARTITIONS; ++i) {
                    int i = 0;
                    bounds[i].minCoord = min(bounds[i].minCoord, data.lightTexCoord);
                    bounds[i].maxCoord = max(bounds[i].maxCoord, data.lightTexCoord);
                //}
                
            }*/
            
            // Drop samples that fall outside the view frustum (clear color, etc)
            // Note: in view space z values are negative and thus nearZ is greater 
            // than farZ!
            //if (data.depth >= nearZ && data.depth <= farZ) {
            const float viewZ = data.positionView.z;
            if (viewZ <= nearZ) { //&& viewZ <= farZ
                uint index = 0;
                for (uint i = 0; i < (PARTITIONS - 1); ++i) {
                    if (viewZ <= shader_data.partitions[i].intervalEnd) {
                        ++index;
                    }
                }
                
                //abs(data.lightTexCoord.z)
                const vec3 toWrite = vec3(data.lightTexCoord.xy, data.depth);
                
                bounds[index].minCoord = min(bounds[index].minCoord, toWrite);
                bounds[index].maxCoord = max(bounds[index].maxCoord, toWrite);
            }
            
        }
    }
    
    const uint groupIndex = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    
    for (uint i = 0; i < PARTITIONS; ++i) {
        uint index = PARTITIONS * groupIndex + i;
        groupMinValues[index] = bounds[i].minCoord;
        groupMaxValues[index] = bounds[i].maxCoord;
    }
    
    
    
    groupMemoryBarrier();
    barrier();
    
    for (uint offset = (REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE >> 1); offset >= PARTITIONS; offset >>= 1) {
        
        for (uint i = groupIndex; i < offset; i += (GROUP_NUM_X*GROUP_NUM_Y)) {
            groupMinValues[i] = min(groupMinValues[i], groupMinValues[i + offset]);
            groupMaxValues[i] = max(groupMaxValues[i], groupMaxValues[i + offset]);
        }
        
        groupMemoryBarrier();
        barrier();
    }
    
    if (groupIndex < PARTITIONS) {
        
        // Using atomic min/max is much faster than using a spin lock
        // This approach assumes that the values are >= 0 ; otherwise correct number order isn't guaranteed!
        // As we mapped the coordinates to lightspace texture space to [0,1] this approach is valid        
        atomicMin(writeOut.results[groupIndex].minCoord.x,  floatBitsToUint(groupMinValues[groupIndex].x));
        atomicMin(writeOut.results[groupIndex].minCoord.y,  floatBitsToUint(groupMinValues[groupIndex].y));
        atomicMin(writeOut.results[groupIndex].minCoord.z,  floatBitsToUint(groupMinValues[groupIndex].z));
        
        atomicMax(writeOut.results[groupIndex].maxCoord.x,  floatBitsToUint(groupMaxValues[groupIndex].x));
        atomicMax(writeOut.results[groupIndex].maxCoord.y,  floatBitsToUint(groupMaxValues[groupIndex].y));
        atomicMax(writeOut.results[groupIndex].maxCoord.z,  floatBitsToUint(groupMaxValues[groupIndex].z));
        
        /*atomicMin(writeOut.results[groupIndex].minCoord.x,  floatBitsToUint(0.0));
        atomicMin(writeOut.results[groupIndex].minCoord.y,  floatBitsToUint(0.0));
        atomicMin(writeOut.results[groupIndex].minCoord.z,  floatBitsToUint(0.0));
        
        atomicMax(writeOut.results[groupIndex].maxCoord.x,  floatBitsToUint(1.0));
        atomicMax(writeOut.results[groupIndex].maxCoord.y,  floatBitsToUint(1.0));
        atomicMax(writeOut.results[groupIndex].maxCoord.z,  floatBitsToUint(1.0));*/
        
        //atomicMin(writeOut.maxResult.x,  floatBitsToUint(0.0));
        
        
        // TODO replace with faster algorithm using atomics, that handles float -> unsigned reinterpretation
        // correctly.
        /*takeMinMaxLock();
        
        writeOut.minResult = min(writeOut.minResult, groupMinValues[groupIndex]);
        writeOut.maxResult = max(writeOut.maxResult, groupMaxValues[groupIndex]);
        
        releaseMinMaxLock();*/
        
        //atomicMax(writeOut.maxResult.x, -100);
    }
}