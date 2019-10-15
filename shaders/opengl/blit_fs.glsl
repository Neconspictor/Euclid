#version 330 core

in VS_OUT {
    vec2 texCoord;
} fs_in;

layout(binding = 0) uniform sampler2D colorMap;

#ifndef USE_STENCIL_TEST
#define USE_STENCIL_TEST 0
#endif

#if USE_STENCIL_TEST
layout(binding = 1) uniform sampler2D stencilMap;
#endif

layout(location = 0) out vec4 fragColor;


void main()
{ 

    #if USE_STENCIL_TEST
        float stencil = texture(stencilMap, fs_in.texCoord).r;
        
        // if stencil isn't set we don't want to blit
        if (stencil < 0.01) {
            discard;
        }    
    #endif

    fragColor = texture(colorMap, fs_in.texCoord);
}