/**
 * Calculates the minimum and maximum z-value of the scene (seen from the camera)  
 */
#version 430 core

#define TILE_WIDTH 16

#define TILE_HEIGHT 64

layout (local_size_x = TILE_WIDTH, local_size_y = 1) in;
// An image to store data into.
//layout (rg32f, binding = 0) uniform image2D data;

layout (binding = 0) uniform sampler2D depthMap;
layout (binding = 1) uniform usampler2D stencilMap;

layout(r32i, binding = 0) uniform iimage1D waterMinDepths;


uniform mat4 inverseViewProjMatrix;


vec3 computeWorldPositionFromDepth(in vec2 texCoord, in float depth) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depth * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = inverseViewProjMatrix * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};


void main(void)
{
    float localMin = 1000000000.0;
    
    const ivec2 globalID =  ivec2(gl_WorkGroupID.xy) * ivec2(TILE_WIDTH, TILE_HEIGHT);
    const vec2 texSize = vec2(textureSize(depthMap, 0).xy);
    
    // Note: y component of gl_LocalInvocationID is always zero
    const ivec2 tileStart = globalID + ivec2(gl_LocalInvocationID.x, 0); 

    
    for (uint tileY = 0; tileY < TILE_HEIGHT; ++tileY) { //TODO stepping
        ivec2 positionScreen = tileStart + ivec2(0, tileY);
        const float depth = texelFetch(depthMap, positionScreen, 0).r;
        const uint stencil = texelFetch(stencilMap, positionScreen, 0).r;
        
        if (stencil == 1) {
            vec2 texCoord = vec2(positionScreen) / texSize;
            vec3 positionWorld = computeWorldPositionFromDepth(texCoord, depth);
        
            localMin = min(localMin, positionWorld.y);
        }
    }

    // Using atomic min/max is much faster than using a spin lock
    // This approach assumes that the values are >= 0 ; otherwise correct number order isn't guaranteed!
    //float normalizedDepth = 0.5 * localMin + 0.5;
    imageAtomicMin(waterMinDepths, tileStart.x, floatBitsToInt(localMin));
}