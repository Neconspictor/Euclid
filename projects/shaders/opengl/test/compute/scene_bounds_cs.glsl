/**
 * Calculates the minimum and maximum z-value of the scene (seen from the camera)  
 */
#version 460 core

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

layout (binding = 0) uniform sampler2D depthTexture;


layout(std430, binding = 0) buffer BufferData // readonly
{
    // Holds positive(!) camera view near and far z-value
    // z and w component aren't used
    vec4 mCameraNearFar;  
    mat4 mCameraProj;
} shader_data;

layout(std430, binding = 1) buffer BufferObject
{
    uvec4 minMax; // x and y component will hold the min and max z value; the other components are not used
} writeOut;

shared float groupMinValues[REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE];
shared float groupMaxValues[REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE];


float computePositiveViewSpaceZ(ivec2 positionScreen) {

    float depth = texelFetch(depthTexture, positionScreen, 0).r; //;imageLoad(depthTexture, positionScreen).r;      
    vec2 gBufferDim = vec2(textureSize(depthTexture, 0));
      
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
    
    //return depth;
    
    // Unproject depth z value into view space
    float z_ndc = 2.0 * depth - 1.0;
    
    // NOTE: view space z might be negative, but we need to return it's positive equivalent.
    return  abs(shader_data.mCameraProj[3][2] / (z_ndc + shader_data.mCameraProj[2][2])); //TODO       
}


void main(void)
{
    float localMin = 999999999.0;
    float localMax = 0;
    
    const ivec2 globalID =  ivec2(gl_WorkGroupID.xy) * ivec2(REDUCE_TILE_WIDTH, REDUCE_TILE_HEIGHT);
    const ivec2 tileStart = globalID + ivec2(gl_LocalInvocationID.xy);
    
    const float nearZ = shader_data.mCameraNearFar.x;
    const float farZ = shader_data.mCameraNearFar.y;
                          
    
    for (uint tileY = 0; tileY < REDUCE_TILE_HEIGHT; tileY += REDUCE_BOUNDS_BLOCK_Y) {
        for (uint tileX = 0; tileX < REDUCE_TILE_WIDTH; tileX += REDUCE_BOUNDS_BLOCK_X) {
        
            ivec2 positionScreen = tileStart + ivec2(tileX, tileY);
            const float viewZ = computePositiveViewSpaceZ(positionScreen);
            if (viewZ > nearZ && viewZ < farZ) {
                localMin = min(localMin, viewZ);
                localMax = max(localMax, viewZ);
            }
        }
    }
    
    const uint groupIndex = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
    
    groupMinValues[groupIndex] = localMin;
    groupMaxValues[groupIndex] = localMax;
    
    groupMemoryBarrier();
    barrier();
    
    for (uint offset = (REDUCE_BOUNDS_SHARED_MEMORY_ARRAY_SIZE >> 1); offset >= 1; offset >>= 1) {
        
        for (uint i = groupIndex; i < offset; i += (GROUP_NUM_X*GROUP_NUM_Y)) {
            groupMinValues[i] = min(groupMinValues[i], groupMinValues[i + offset]);
            groupMaxValues[i] = max(groupMaxValues[i], groupMaxValues[i + offset]);
        }
        
        groupMemoryBarrier();
        barrier();
    }
    
    if (groupIndex == 0) {
        // Using atomic min/max is much faster than using a spin lock
        // This approach assumes that the values are >= 0 ; otherwise correct number order isn't guaranteed!
        // This is the reason for using positive view space z values!
        atomicMin(writeOut.minMax.x,  floatBitsToUint(groupMinValues[groupIndex]));        
        atomicMax(writeOut.minMax.y,  floatBitsToUint(groupMaxValues[groupIndex]));
    }
}