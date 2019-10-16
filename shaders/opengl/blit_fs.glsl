#version 330 core

in VS_OUT {
    vec2 texCoord;
} fs_in;

layout(binding = 0) uniform sampler2D colorMap;
layout(binding = 1) uniform sampler2D depthMap;

#ifndef USE_STENCIL_TEST
#define USE_STENCIL_TEST 0
#endif

#if USE_STENCIL_TEST
layout(binding = 2) uniform usampler2D stencilMap;
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
    luminance = vec4(0.0);
    
    gl_FragDepth = texture(depthMap, fs_in.texCoord).r;
}