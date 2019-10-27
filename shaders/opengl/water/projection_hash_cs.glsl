#version 450 core

layout (local_size_x = 1, local_size_y = 1) in;

layout (binding = 0) uniform sampler2D depthMap;

// writeonly
layout (r32ui, binding = 0) uniform uimage2D projHashMap;

uniform mat4 viewProj;
uniform mat4 invViewProj;
uniform vec2 texSize;
uniform float waterHeight;


/**
 * projects a world space position into texture space
 */
vec2 project(in vec3 positionWS) {
    vec4 positionCS = viewProj * vec4(positionWS, 1.0);
    vec3 positionNDC = positionCS.xyz / positionCS.w;
    
    //NDC is in range [-1, 1], but we need [0,1]
    return (0.5 * positionNDC.xy + 0.5);
}

/**
 * Computes world position of texture coords and depth
 */
vec3 unproject(in vec2 texCoords, float depth) {
    vec4 positionCS;
    positionCS.xy = 2.0 * texCoords - vec2(1.0);
    positionCS.z = 2.0 * depth - 1.0;
    positionCS.w = 1.0;
    
    vec4 position = invViewProj * positionCS;
    
    // perspective division has to be reversed
    return position.xyz / position.w;
}

void main(void)
{
    const vec2 size = vec2(textureSize(depthMap, 0));
    const ivec2 pixelLoc = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
    const uvec2 uPixelLoc = uvec2(pixelLoc);
    
    const vec2 texCoords = vec2(pixelLoc) / size;

    const float depth = texelFetch(depthMap, pixelLoc, 0).r;
    
    vec3 positionWS = unproject(texCoords, depth);
    
    float reflectedY = 2.0 * waterHeight - positionWS.y;
    float distY = positionWS.y - waterHeight;
    
    
    // reflect on water plane
    vec3 reflectedWs = vec3(positionWS.x, reflectedY, positionWS.z);
    
    vec2 reflectedUV = project(reflectedWs);
    ivec2 reflectedPixelLoc = ivec2(reflectedUV * size);
    
     uint hash = 0;
    
    if (reflectedPixelLoc.x <= size.x && reflectedPixelLoc.y <= size.y && (reflectedY < waterHeight)) {
        //encode the source pixel location in a single 32 bit unsigned
        hash = (uint(size.y) - uPixelLoc.y) << 16 | uPixelLoc.x;
    }
    
    
    
    // Needed since multiple source pixels can be mapped to the same reflected position
    // We want to have win the nearest source pixel -> use atomic max
    //imageStore(projHashMap, reflectedPixelLoc, uvec4(hash));
    imageAtomicMax(projHashMap, reflectedPixelLoc, hash);
}