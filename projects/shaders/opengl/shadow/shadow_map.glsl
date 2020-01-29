/**
 * The following prerequisites should be defined before including this file: 
 *  + SHADOW_DEPTH_MAP_BINDING_POINT (default is 0)
 */

#ifndef SHADOW_SAMPLE_COUNT_X
#define SHADOW_SAMPLE_COUNT_X 2
#endif

#ifndef SHADOW_SAMPLE_COUNT_Y
#define SHADOW_SAMPLE_COUNT_Y 2
#endif

#ifndef SHADOW_USE_LERP_FILTER
#define SHADOW_USE_LERP_FILTER 1
#endif

#ifndef SHADOW_ENABLED
#define SHADOW_ENABLED 1
#endif

#ifndef SHADOW_BIAS_MULTIPLIER
#define SHADOW_BIAS_MULTIPLIER 6.0
#endif

#ifndef SHADOW_DEPTH_MAP_BINDING_POINT
#define SHADOW_DEPTH_MAP_BINDING_POINT 0
#endif


#ifndef USE_ARRAY_SAMPLER
#define USE_ARRAY_SAMPLER 0
#endif

#ifndef USE_SHADOW_SAMPLER
#define USE_SHADOW_SAMPLER 0
#endif

#include "shadow/shadows_common.glsl"


layout(binding = SHADOW_DEPTH_MAP_BINDING_POINT) uniform sampler2D shadowMap;

uniform mat4 lightViewProjectionMatrix;


float computeShadow(const in vec3 lightDirectionWorld, 
                     const in vec3 normalWorld,
                     const in vec3 worldPosition)
{	

#if SHADOW_ENABLED == 0

#else

	float sDotN = dot(lightDirectionWorld, normalWorld);
	
	// assure that fragments with a normal facing away from the light source 
	// are always in shadow (reduces unwanted unshadowing).
	if (sDotN < 0) {
		return 0;
	}
    
	float angleBias = 0.006f;
	vec4 fragmentModelPosition = vec4(worldPosition, 1.0);
	vec4 fragmentShadowPosition = lightViewProjectionMatrix * fragmentModelPosition;
	vec2 projCoords = fragmentShadowPosition.xy;

	//Remap the -1 to 1 NDC to the range of 0 to 1
	projCoords.xy = projCoords.xy * 0.5f + 0.5f;

	// Get depth of current fragment from light's perspective
    //float currentDepth = 0.5 * fragmentShadowPosition.z + 0.5;
	float currentDepth = fragmentShadowPosition.z;

	float bias = max(angleBias * (1.0 - dot(normalWorld, lightDirectionWorld)), 0.0008);
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
	float minBias = max(texelSize.x,texelSize.y);
	bias =  SHADOW_BIAS_MULTIPLIER * minBias;
    
    
	float shadow = 0.0;
	
	float sampleCount = (2 * SHADOW_SAMPLE_COUNT_X + 1) * (2 * SHADOW_SAMPLE_COUNT_Y + 1);
	
	float penumbraSize = 1.0;
	vec2 size = textureSize(shadowMap, 0).xy;
		
    for(float x= -SHADOW_SAMPLE_COUNT_X; x <= SHADOW_SAMPLE_COUNT_X; x += 1){
        for(float y = -SHADOW_SAMPLE_COUNT_Y; y <= SHADOW_SAMPLE_COUNT_Y; y += 1){
            vec2 off = vec2(x,y)/size * penumbraSize;
			vec2 uv = projCoords + off;
			float compare = currentDepth - bias;
            
            #if SHADOW_USE_LERP_FILTER
                float shadowSample =  shadowLerp(shadowMap, size, uv, 0, currentDepth, bias, penumbraSize);
            #else
                float shadowSample = shadowCompare(shadowMap, vec4(uv, 0, currentDepth - bias));
            #endif
            
            
            shadow += shadowSample;
        }
    }
    
    return shadow / (sampleCount);
    
#endif
}