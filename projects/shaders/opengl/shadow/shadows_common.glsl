#extension GL_EXT_texture_array : enable

#ifndef USE_ARRAY_SAMPLER
#define USE_ARRAY_SAMPLER 0
#endif

#ifndef USE_SHADOW_SAMPLER
#define USE_SHADOW_SAMPLER 0
#endif

#if USE_ARRAY_SAMPLER

    #if USE_SHADOW_SAMPLER
        #ifndef BASE_SAMPLER_TYPE
        #define BASE_SAMPLER_TYPE sampler2DArrayShadow
        #endif
    #else
        #ifndef BASE_SAMPLER_TYPE
        #define BASE_SAMPLER_TYPE sampler2DArray
        #endif
    #endif

#else

    #if USE_SHADOW_SAMPLER
        #ifndef SHADOW_SAMPLER_TYPE
        #define SHADOW_SAMPLER_TYPE sampler2DShadow
        #endif
    #else
        #ifndef BASE_SAMPLER_TYPE
        #define BASE_SAMPLER_TYPE sampler2D
        #endif    
    #endif

#endif



/**
 * Checks if compare is in range [min, max]
 * Returns 1.0 if compare is in the forementioned range.
 * Otherwise 0.0 is returned.
 */
float isInRange(float min, float max, float compare) {
    return step(min, compare) - step(max, compare);
};

float shadowCompare(BASE_SAMPLER_TYPE depths, vec4 uvZCompareBias){

    #if USE_ARRAY_SAMPLER && USE_SHADOW_SAMPLER
        return shadow2DArray(depths, uvZCompareBias.xyzw).r;
    #elif USE_SHADOW_SAMPLER
        return shadow2D(depths, uvZCompareBias.xyw).r;
    #else
        
        #if USE_ARRAY_SAMPLER
            float depth = texture(depths, uvZCompareBias.xyz).r;
        #else
            float depth = texture(depths, uvZCompareBias.xy).r;
        #endif
        
        return step(uvZCompareBias.w, depth);
    
    #endif
}

float shadowLerp(BASE_SAMPLER_TYPE depths, vec2 size, vec2 uv, float arrayIndex, float compare, float bias, float penumbraSize){
    vec2 texelSize = vec2(1.0)/size;
    vec2 f = fract(uv*size+0.5);
    vec2 centroidUV = (uv*size+0.5)/size;

    float lb = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(0.0, 0.0), arrayIndex, compare - bias));
	float lt = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(0.0, penumbraSize), arrayIndex, compare - bias));
	float rb = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(penumbraSize, 0.0), arrayIndex, compare - bias));
	float rt = shadowCompare(depths, vec4(centroidUV +texelSize*vec2(penumbraSize, penumbraSize), arrayIndex, compare - bias));
	float a = mix(lb, lt, f.y);
	float b = mix(rb, rt, f.y);
    float c = mix(a, b, f.x);
    return c;
}