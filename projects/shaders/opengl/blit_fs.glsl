#version 460 core

//#extension GL_ARB_shader_stencil_export : require //NOTE: Not supported 

#ifndef BLIT_COLOR0
#define BLIT_COLOR0 0
#endif

#ifndef BLIT_COLOR1
#define BLIT_COLOR1 0
#endif

#ifndef BLIT_DEPTH
#define BLIT_DEPTH 0
#endif

#ifndef USE_STENCIL_TEST_FOR_BLITTING
#define USE_STENCIL_TEST_FOR_BLITTING 0
#endif

in VS_OUT {
    vec2 texCoord;
} fs_in;

#if BLIT_COLOR0
layout(binding = 0) uniform sampler2D colorMap0;
#endif

#if BLIT_COLOR1
layout(binding = 1) uniform sampler2D colorMap1;
#endif

#if BLIT_DEPTH
layout(binding = 2) uniform sampler2D depthMap;
#endif


#if USE_STENCIL_TEST_FOR_BLITTING
layout(binding = 3) uniform usampler2D stencilMap;
#endif

#if BLIT_COLOR0
layout(location = 0) out vec4 colorOut0;
#endif

#if BLIT_COLOR1
layout(location = 1) out vec4 colorOut1;
#endif

void main()
{ 

    #if USE_STENCIL_TEST_FOR_BLITTING
        uint stencil = texture(stencilMap, fs_in.texCoord).r;
        
        // if stencil isn't set we don't want to blit
        if (stencil < 1) {
            discard;
        }    
    #endif

	#if BLIT_COLOR0
		colorOut0 = texture(colorMap0, fs_in.texCoord);
    #endif
	
	
    #if BLIT_COLOR1
        colorOut1 = texture(colorMap1, fs_in.texCoord);
		colorOut1 = vec4(0,0,0,1);
    #endif
    
    #if BLIT_DEPTH
    //gl_FragStencilRefARB = 1;
	gl_FragDepth = texture(depthMap, fs_in.texCoord).r;
    #endif
}