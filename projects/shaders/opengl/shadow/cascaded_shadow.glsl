/**
 * The following prerequisites should be defined before including this file: 
 *  + A buffer named constants with a CascadeData element
 *  + CSM_CASCADE_DEPTH_MAP_BINDING_POINT (default is 0)
 */
 
#ifndef CSM_NUM_CASCADES
#define CSM_NUM_CASCADES 4
#endif

#ifndef USE_ARRAY_SAMPLER
#define USE_ARRAY_SAMPLER 1
#endif

#ifndef USE_SHADOW_SAMPLER
#define USE_SHADOW_SAMPLER 0
#endif

#include "shadow/shadows_common.glsl"
#include "interface/shadow/cascade_common.h"

#ifndef CSM_SAMPLE_COUNT_X
#define CSM_SAMPLE_COUNT_X 2
#endif

#ifndef CSM_SAMPLE_COUNT_Y
#define CSM_SAMPLE_COUNT_Y 2
#endif

#ifndef CSM_USE_LERP_FILTER
#define CSM_USE_LERP_FILTER 1
#endif

#ifndef CSM_ENABLED
#define CSM_ENABLED 1
#endif

#ifndef CSM_BIAS_MULTIPLIER
#define CSM_BIAS_MULTIPLIER 9.0
#endif

#ifndef CSM_CASCADE_DEPTH_MAP_BINDING_POINT
#define CSM_CASCADE_DEPTH_MAP_BINDING_POINT 0
#endif

#include "util/depth_util.glsl"


layout(binding = CSM_CASCADE_DEPTH_MAP_BINDING_POINT) uniform sampler2DArray cascadedDepthMap;


uint getCascadeIdx(in float viewSpaceZ) {
    uint cascadeIdx = 0;
    
    const float positiveZ = -viewSpaceZ;

    // Figure out which cascade to sample from
    for(uint i = 0; i < CSM_NUM_CASCADES - 1; ++i)
    {
        if(positiveZ > constants.cascadeData.cascadedSplits[i].x)
        {	
            cascadeIdx = i + 1;
        }
    }
    
    return cascadeIdx;
};

/**
 * NOTE: lightDirection and normal can be in any space, they only have to be in the same space.
 */
float cascadedShadow(const in vec3 lightDirection, 
                     const in vec3 normal, 
                     const in float depthViewSpace,
                     const in vec3 viewPosition)
{

#if CSM_ENABLED == 0
    // sample is full lit
    return 1.0f;
#else

	
	float sDotN = dot(lightDirection, normal);
	
	// assure that fragments with a normal facing away from the light source 
	// are always in shadow (reduces unwanted unshadowing).
	if (sDotN < 0) {
		//return 0;
	}
	
    // Figure out which cascade to sample from
	uint cascadeIdx = getCascadeIdx(depthViewSpace);
    
	float angleBias = 0.006f;

	mat4 lightViewProjectionMatrix = constants.cascadeData.lightViewProjectionMatrices[cascadeIdx];

	vec4 fragmentModelViewPosition = vec4(viewPosition,1.0f);

	vec4 fragmentModelPosition = constants.invViewGPass * fragmentModelViewPosition;

	vec4 fragmentShadowPosition = lightViewProjectionMatrix * fragmentModelPosition;

	vec3 projCoords = fragmentShadowPosition.xyz /= fragmentShadowPosition.w;

	//Remap from NDC to screen space
	projCoords.xy = projCoords.xy * 0.5f + 0.5f;

	//projCoords.z = 0.5 * projCoords.z + 0.5;

	// Get depth of current fragment from light's perspective
    float currentDepth = z_ndcToScreen(projCoords.z);
	projCoords.z = cascadeIdx;   

	float bias = max(angleBias * (1.0 - dot(normal, lightDirection)), 0.0008);
	//vec2 texelSize = vec2(1.0)/textureSize(cascadedDepthMap, 0);
	vec2 texelSize = 1.0 / textureSize(cascadedDepthMap, 0).xy;
	float minBias = max(texelSize.x,texelSize.y);
	bias =  CSM_BIAS_MULTIPLIER * minBias / constants.cascadeData.scaleFactors[cascadeIdx].x;
    //bias = minBias;

	float shadow = 0.0;
	//vec2 texelSize = 1.0 / textureSize(cascadedDepthMap, 0).xy;
	
	float sampleCount = (2*CSM_SAMPLE_COUNT_X + 1) * (2*CSM_SAMPLE_COUNT_Y + 1);
	
	/*float depth = texture2DArray(cascadedDepthMap, vec3(projCoords.xy, projCoords.z)).r;
	float diff =  abs(currentDepth - depth);
	float penumbraSize = diff / (minBias);
	penumbraSize = clamp(penumbraSize, 0, 1);
	penumbraSize = (CSM_NUM_CASCADES - projCoords.z) / CSM_NUM_CASCADES;
	penumbraSize = CSM_NUM_CASCADES * projCoords.z;*/
	float penumbraSize = 1.0;
	vec2 size = textureSize(cascadedDepthMap, 0).xy;
	
		
    for(float x=-CSM_SAMPLE_COUNT_X; x<=CSM_SAMPLE_COUNT_X; x += 1){
        for(float y=-CSM_SAMPLE_COUNT_Y; y<=CSM_SAMPLE_COUNT_Y; y += 1){
            vec2 off = vec2(x,y)/size * penumbraSize;
			vec2 uv = projCoords.xy + off;
			float compare = currentDepth - bias;
            
            #if CSM_USE_LERP_FILTER
                float shadowSample =  shadowLerp(cascadedDepthMap, size, uv, projCoords.z, currentDepth, bias, penumbraSize);
            #else
                float shadowSample = shadowCompare(cascadedDepthMap, vec4(uv, projCoords.z, currentDepth - bias));
            #endif
            
            
            shadow += shadowSample;
        }
    }
    
    return shadow / (sampleCount);

	//float pcfDepth =  shadow2DArray(cascadedDepthMap, vec4(projCoords.xyz, currentDepth + bias )).r; 
	//shadow += currentDepth  > pcfDepth ? 0.0  : 1.0;
	//return shadow; 
#endif    
}



float indexedShadow(const in vec3 lightDirectionWorld, 
                     const in vec3 normalWorld, 
                     const in uint cascadeIdx,
                     const in vec3 worldPosition)
{	

#if CSM_ENABLED == 0
		// sample is full lit
		return 1.0f;
#else

	float sDotN = dot(lightDirectionWorld, normalWorld);
	
	// assure that fragments with a normal facing away from the light source 
	// are always in shadow (reduces unwanted unshadowing).
	if (sDotN < 0) {
		return 0;
	}
    
	float angleBias = 0.006f;

	mat4 lightViewProjectionMatrix = constants.cascadeData.lightViewProjectionMatrices[cascadeIdx];
	vec4 fragmentModelPosition = vec4(worldPosition, 1.0);

	vec4 fragmentShadowPosition = lightViewProjectionMatrix * fragmentModelPosition;

	vec3 projCoords = fragmentShadowPosition.xyz /= fragmentShadowPosition.w;

	//Remap the -1 to 1 NDC to the range of 0 to 1
	projCoords.xyz = projCoords.xyz * 0.5f + 0.5f;
	//projCoords.z = 2.0 * projCoords.z - 1.0;

	// Get depth of current fragment from light's perspective
    float currentDepth = z_ndcToScreen(projCoords.z);

	projCoords.z = cascadeIdx;   

	float bias = max(angleBias * (1.0 - dot(normalWorld, lightDirectionWorld)), 0.0008);
	vec2 texelSize = 1.0 / textureSize(cascadedDepthMap, 0).xy;
	float minBias = max(texelSize.x,texelSize.y);
	bias =  CSM_BIAS_MULTIPLIER * minBias / constants.cascadeData.scaleFactors[cascadeIdx].x;

	float shadow = 0.0;
	
	float sampleCount = (2*CSM_SAMPLE_COUNT_X + 1) * (2*CSM_SAMPLE_COUNT_Y + 1);
	
	float penumbraSize = 1.0;
	vec2 size = textureSize(cascadedDepthMap, 0).xy;
	
		
    for(float x=-CSM_SAMPLE_COUNT_X; x<=CSM_SAMPLE_COUNT_X; x += 1){
        for(float y=-CSM_SAMPLE_COUNT_Y; y<=CSM_SAMPLE_COUNT_Y; y += 1){
            vec2 off = vec2(x,y)/size * penumbraSize;
			vec2 uv = projCoords.xy + off;
			float compare = currentDepth - bias;
            
            #if CSM_USE_LERP_FILTER
                float shadowSample =  shadowLerp(cascadedDepthMap, size, uv, projCoords.z, currentDepth, bias, penumbraSize);
            #else
                float shadowSample = shadowCompare(cascadedDepthMap, vec4(uv, projCoords.z, currentDepth - bias));
            #endif
            
            
            shadow += shadowSample;
        }
    }
    
    return shadow / (sampleCount);
#endif
}