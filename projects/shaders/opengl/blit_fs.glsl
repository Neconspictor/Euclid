#version 460 core

#ifndef USE_LUMINANCE
#define USE_LUMINANCE 0
#endif

#ifndef USE_DEPTH
#define USE_DEPTH 0
#endif

#ifndef USE_STENCIL_TEST
#define USE_STENCIL_TEST 0
#endif

in VS_OUT {
    vec2 texCoord;
} fs_in;

layout(binding = 0) uniform sampler2D colorMap;

#if USE_LUMINANCE
layout(binding = 1) uniform sampler2D luminanceMap;
#endif

#if USE_DEPTH
layout(binding = 2) uniform sampler2D depthMap;
#endif


#if USE_STENCIL_TEST
layout(binding = 3) uniform usampler2D stencilMap;
#endif

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 luminance;


void main()
{ 

    #if USE_STENCIL_TEST
        uint stencil = texture(stencilMap, fs_in.texCoord).r;
        
        // if stencil isn't set we don't want to blit
        if (stencil < 1) {
            discard;
        }    
    #endif

    fragColor = texture(colorMap, fs_in.texCoord);
    
    #if USE_LUMINANCE
        luminance = texture(luminanceMap, fs_in.texCoord);
    #endif
    
    #if USE_DEPTH
        gl_FragDepth = texture(depthMap, fs_in.texCoord).r;
    #endif
}