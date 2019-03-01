#include "shadow/shadows_array.glsl"

#ifndef CSM_NUM_CASCADES
#define CSM_NUM_CASCADES 4
#endif

#ifndef CSM_SAMPLE_COUNT_X
#define CSM_SAMPLE_COUNT_X 0
#endif

#ifndef CSM_SAMPLE_COUNT_Y
#define CSM_SAMPLE_COUNT_Y 0
#endif

#ifndef CSM_USE_LERP_FILTER
#define CSM_USE_LERP_FILTER 0
#endif




struct CascadeData {
	//mat4 viewMatrix;
	mat4 inverseViewMatrix;
	mat4 lightViewProjectionMatrices[CSM_NUM_CASCADES];
    vec4 scaleFactors[CSM_NUM_CASCADES];
	vec4 cascadedSplits[CSM_NUM_CASCADES];
};

uint getCascadeIdx(in float viewSpaceZ, in CascadeData cascadeData) {
    uint cascadeIdx = 0;
    
    const float positiveZ = -viewSpaceZ;

    // Figure out which cascade to sample from
    for(uint i = 0; i < CSM_NUM_CASCADES - 1; ++i)
    {
        if(positiveZ > cascadeData.cascadedSplits[i].x)
        {	
            cascadeIdx = i + 1;
        }
    }
    
    return cascadeIdx;
};


float cascadedShadow(in vec3 lightDirection, 
                     in vec3 normal, 
                     in float depthViewSpace,
                     in vec3 viewPosition,
                     in CascadeData cascadeData,
                     in sampler2DArray cascadedDepthMap)
{
	
	float sDotN = dot(lightDirection, normal);
	
	// assure that fragments with a normal facing away from the light source 
	// are always in shadow (reduces unwanted unshadowing).
	if (sDotN < 0) {
		return 0;
	}
	
    // Figure out which cascade to sample from
	uint cascadeIdx = getCascadeIdx(depthViewSpace, cascadeData);
    
	float angleBias = 0.006f;

	mat4 lightViewProjectionMatrix = cascadeData.lightViewProjectionMatrices[cascadeIdx];

	vec4 fragmentModelViewPosition = vec4(viewPosition,1.0f);

	vec4 fragmentModelPosition = cascadeData.inverseViewMatrix * fragmentModelViewPosition;

	vec4 fragmentShadowPosition = lightViewProjectionMatrix * fragmentModelPosition;

	vec3 projCoords = fragmentShadowPosition.xyz /= fragmentShadowPosition.w;

	//Remap the -1 to 1 NDC to the range of 0 to 1
	projCoords = projCoords * 0.5f + 0.5f;

	// Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

	projCoords.z = cascadeIdx;   

	float bias = max(angleBias * (1.0 - dot(normal, lightDirection)), 0.0008);
	//vec2 texelSize = vec2(1.0)/textureSize(cascadedDepthMap, 0);
	vec2 texelSize = 1.0 / textureSize(cascadedDepthMap, 0).xy;
	float minBias = max(texelSize.x,texelSize.y);
	bias =  9 * minBias / cascadeData.scaleFactors[cascadeIdx].x;
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
}