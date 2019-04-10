#version 430 core

//#extension GL_EXT_texture_array : enable
#extension GL_NV_texture_array : enable

#ifndef CASCADE_COMMON_INCLUDE
#define CASCADE_COMMON_INCLUDE

#ifndef CSM_NUM_CASCADES
#define CSM_NUM_CASCADES 4
#endif

struct CascadeData {
	//mat4 viewMatrix;
	mat4 inverseViewMatrix;
    //mat4 worldToShadowSpace;
   //mat4 viewToShadowSpace;
	mat4 lightViewProjectionMatrices[CSM_NUM_CASCADES];
    vec4 scaleFactors[CSM_NUM_CASCADES];
	vec4 cascadedSplits[CSM_NUM_CASCADES];
};

#endif //CASCADE_COMMON_INCLUDE

#ifndef CSM_SAMPLE_COUNT_X
#define CSM_SAMPLE_COUNT_X 0
#endif

#ifndef CSM_SAMPLE_COUNT_Y
#define CSM_SAMPLE_COUNT_Y 0
#endif

#ifndef CSM_USE_LERP_FILTER
#define CSM_USE_LERP_FILTER 0
#endif

#ifndef CSM_ENABLED
#define CSM_ENABLED 1
#endif

#ifndef CSM_BIAS_MULTIPLIER
#define CSM_BIAS_MULTIPLIER 9.0
#endif


layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;

in VS_OUT {	
	vec2 tex_coords;
} fs_in;

struct GBuffer {
    layout(binding = 0) sampler2D albedoMap;
    layout(binding = 1)	sampler2D aoMetalRoughnessMap;
    layout(binding = 2)	sampler2D normalEyeMap;
    layout(binding = 3) sampler2D normalizedViewSpaceZMap;
};

uniform GBuffer gBuffer;
//layout(binding = 0) uniform sampler2D albedoMap;
//layout(binding = 1) uniform sampler2D aoMetalRoughnessMap;
//layout(binding = 2) uniform sampler2D normalEyeMap;
//layout(binding = 3) uniform sampler2D normalizedViewSpaceZMap;

// IBL
layout(binding = 5) uniform samplerCube irradianceMap;
layout(binding = 6) uniform samplerCube prefilterMap;
layout(binding = 7) uniform sampler2D brdfLUT;

//uniform mat4 inverseViewMatrix_GPass; // the inverse view from the geometry pass!
uniform mat4 inverseProjMatrix_GPass;
uniform vec2 nearFarPlane;


const float PI = 3.14159265359;

struct DirLight {
    vec3 directionEye;
    vec3 color;
    float power;
};

uniform DirLight dirLight;

uniform float ambientLightPower;
uniform float shadowStrength;

// The inverse view matrix. Note, for deferred renderings the inverse view of the geometry pass is meant!
uniform mat4 inverseViewMatrix;


// Cascaded shadow mapping
layout(std140,binding=0) buffer CascadeBuffer { //buffer uniform
	/*mat4 inverseViewMatrix;
	mat4 lightViewProjectionMatrices[CSM_NUM_CASCADES];
    vec4 scaleFactors[CSM_NUM_CASCADES];
	vec4 cascadedSplits[CSM_NUM_CASCADES];*/
    CascadeData cascadeData;
} csmData;

layout(binding = 8) uniform sampler2DArray cascadedDepthMap;


/**
 * Checks if compare is in range [min, max]
 * Returns 1.0 if compare is in the forementioned range.
 * Otherwise 0.0 is returned.
 */
float isInRange(float min, float max, float compare) {
    return step(min, compare) - step(max, compare);
};

float shadowCompare(vec4 uvZCompareBias){
    float depth = texture2DArray(cascadedDepthMap, uvZCompareBias.xyz).r;
    //return depth;
	return step(uvZCompareBias.w, depth);
}

float shadowLerp(vec2 size, vec2 uv, float projectedZ, float compare, float bias, float penumbraSize){
    vec2 texelSize = vec2(1.0)/size;
    vec2 f = fract(uv*size+0.5);
    vec2 centroidUV = (uv*size+0.5)/size;

    float lb = shadowCompare(vec4(centroidUV +texelSize*vec2(0.0, 0.0), projectedZ, compare - bias));
	float lt = shadowCompare(vec4(centroidUV +texelSize*vec2(0.0, penumbraSize), projectedZ, compare - bias));
	float rb = shadowCompare(vec4(centroidUV +texelSize*vec2(penumbraSize, 0.0), projectedZ, compare - bias));
	float rt = shadowCompare(vec4(centroidUV +texelSize*vec2(penumbraSize, penumbraSize), projectedZ, compare - bias));
	float a = mix(lb, lt, f.y);
	float b = mix(rb, rt, f.y);
    float c = mix(a, b, f.x);
    
	//return 0.0;
	return c;
	
}



uint getCascadeIdx(float viewSpaceZ) {
    uint cascadeIdx = 0;
    
    const float positiveZ = -viewSpaceZ;

    // Figure out which cascade to sample from
    for(uint i = 0; i < CSM_NUM_CASCADES - 1; ++i)
    {
        if(positiveZ > csmData.cascadeData.cascadedSplits[i].x)
        {	
            cascadeIdx = i + 1;
        }
    }
    
    return cascadeIdx;
};


float cascadedShadow( vec3 lightDirection, 
                      vec3 normal, 
                      float depthViewSpace,
                      vec3 viewPosition)
{

#if CSM_ENABLED == 0
    // sample is full lit
    return 1.0f;
#else

	
	float sDotN = dot(lightDirection, normal);
	
	// assure that fragments with a normal facing away from the light source 
	// are always in shadow (reduces unwanted unshadowing).
	if (sDotN < 0) {
		return 0;
	}
	
    // Figure out which cascade to sample from
	uint cascadeIdx = getCascadeIdx(depthViewSpace);
    
	float angleBias = 0.006f;

	mat4 lightViewProjectionMatrix = csmData.cascadeData.lightViewProjectionMatrices[cascadeIdx];

	vec4 fragmentModelViewPosition = vec4(viewPosition,1.0f);

	vec4 fragmentModelPosition = csmData.cascadeData.inverseViewMatrix * fragmentModelViewPosition;

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
	bias =  CSM_BIAS_MULTIPLIER * minBias / csmData.cascadeData.scaleFactors[cascadeIdx].x;
    //bias = minBias;

	float shadow = 0.0;
	//vec2 texelSize = 1.0 / textureSize(csmData.cascadedDepthMap, 0).xy;
	
	float sampleCount = (2*CSM_SAMPLE_COUNT_X + 1) * (2*CSM_SAMPLE_COUNT_Y + 1);
	
	/*float depth = texture2DArray(csmData.cascadedDepthMap, vec3(projCoords.xy, projCoords.z)).r;
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
                float shadowSample =  shadowLerp(size, uv, projCoords.z, currentDepth, bias, penumbraSize);
            #else
                float shadowSample = shadowCompare(vec4(uv, projCoords.z, currentDepth - bias));
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
#ifndef VIEWSPACE_NORMALIZATION_HEADER
#define VIEWSPACE_NORMALIZATION_HEADER

/**
 * Maps a viewspace z value to the range [0,1]
 * @param z : viewspace z value to be mapped.
 * @param near: viewspace z value of the near plane
 * @param far: viewspace z value of the far plane
 */
float normalizeViewSpaceZ(float z, float near, float far) {    
    const float normalized = (z - near) / (far - near);
    return clamp(normalized, 0.0, 1.0);
}

/**
 * Denormalizes a normalized viewspace z value from the range [0,1] to [near, far].
 * Whereby near and far are the near resp. far plane viewspace z values.
 * The result will be the viewspace z value of the normalized z value.
 * @param normalizedZ: A normalized viewspace z value.
 * @param near: viewspace z value of the near plane
 * @param far: viewspace z value of the far plane
 */
float denormalizeViewSpaceZ(float normalizedZ, float near, float far) {
    return normalizedZ * (far - near) + near;
}

/**
 * Calculates a view space position from a view space z value, a texture coordinate and an inverse camera projection.
 */
vec3 getViewPositionFromNormalizedZ(in vec2 texCoord, in float viewSpaceZ, in mat4 inverseProjection) {
  
  const vec2 texNDC = texCoord * 2.0 - 1.0;
  const vec3 viewSpaceRay = (inverseProjection * vec4(texNDC, 1.0, 1.0)).xyz;
  // Note: we use a distance value. In opengl viewSpaceZ is negative, thus we use the negative 
  return -viewSpaceZ * viewSpaceRay;
};

#endif // VIEWSPACE_NORMALIZATION_HEADER


/*vec3 pbrDirectLight(vec3 V, vec3 N, float roughness, vec3 F0, float metallic, vec3 albedo);
vec3 pbrAmbientLight(vec3 V, vec3 N, vec3 normalWorld, float roughness, vec3 F0, float metallic, vec3 albedo, vec3 reflectionDirWorld, float ao);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);*/




// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   
// ----------------------------------------------------------------------------


vec3 pbrDirectLight(vec3 V, vec3 N, float roughness, vec3 F0, float metallic, vec3 albedo) {
	
    vec3 L = normalize(-dirLight.directionEye);
	vec3 H = normalize(V + L);
	
	// directional lights have no distance and thus also no attenuation
	//float distance = length(lightPosition - fragPos);
	//float attenuation = 1.0 / (distance * distance);
	
	
	//vec3 radiance = vec3(243/ 255.0f, 159 / 255.0f, 24 / 255.0f) * 1.0f;//dirLight.color; /** attenuation*/
	vec3 radiance = dirLight.color * dirLight.power;//dirLight.color; /** attenuation*/

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness);   
	float G   = GeometrySmith(N, V, L, roughness);    
	vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
	
	// specular reflection
	vec3 nominator    = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
	vec3 specular = nominator / denominator;
	
	 // kS is equal to Fresnel
	vec3 kS = F;
	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	vec3 kD = vec3(1.0) - kS;
	// multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - metallic;	                
		
	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);        

	// add to outgoing radiance Lo
	return (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again	
}

vec3 pbrAmbientLight(vec3 V, vec3 N, vec3 normalWorld, float roughness, vec3 F0, float metallic, vec3 albedo, vec3 reflectionDirWorld, float ao) {
	// ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    //Important: We need world space normals! TODO: Maybe it is possible to generate 
    // irridianceMap in such a way, that we can use view space normals, too.
    vec3 irradiance = texture(irradianceMap, normalWorld).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
	
    // Important: R has to be in world space, too.
    vec3 prefilteredColor = textureLod(prefilterMap, reflectionDirWorld, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	//brdf = vec2(1.0, 0.0);
	//brdf = vec2(1,1);
    vec3 ambientLightSpecular = prefilteredColor * (F * brdf.x + brdf.y);

    return ambientLightPower * (kD * diffuse + ambientLightSpecular) * ao;
}




void calcLighting(in float ao, 
             in vec3 albedo, 
             in float metallic, 
             in vec3 normalEye, 
             in float roughness,
             in vec3 positionEye,             
             in vec2 texCoord,
             out vec3 colorOut,
             out vec3 luminanceOut) 
{
    // view direction
	vec3 viewEye = normalize(-positionEye);
    
	// reflection direction
    vec3 viewWorld = vec3(inverseViewMatrix * vec4(viewEye, 0.0f));
    vec3 normalWorld = vec3(inverseViewMatrix * vec4(normalEye, 0.0f));
    vec3 reflectionDirWorld = reflect(-viewWorld, normalWorld);


    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = pbrDirectLight(viewEye, normalEye, roughness, F0, metallic, albedo);
    
    vec3 ambient =  pbrAmbientLight(viewEye, normalEye, normalWorld, roughness, F0, metallic, albedo, reflectionDirWorld, ao);
    
    float fragmentLitProportion = cascadedShadow(-dirLight.directionEye, normalEye, positionEye.z, positionEye);
	//fragmentLitProportion = 0;
	
	//colorOut = vec3(fragmentLitProportion);
    //luminanceOut = colorOut;
	//return;
	
    
	
    vec3 color = ambient; //* ambientShadow; // ssaoAmbientOcclusion;
	//color = vec3(0.0);
    float ambientShadow = clamp(fragmentLitProportion, 1.0 - shadowStrength, 1.0);
    color -= color*(1.0 - ambientShadow);
	
	// shadows affecting only direct light contribution
	//color += Lo * shadow;
    vec3 directLighting = fragmentLitProportion * Lo;
    
	color += directLighting;
    
    colorOut = color;
    luminanceOut = directLighting;
    
    return;
    
    //Debug
    uint cascadeIdx = getCascadeIdx(positionEye.z);
    //cascadeIdx = 10;
    
    vec3 cascadeColor = colorOut;
    
    if (cascadeIdx == 0) {
       cascadeColor = vec3(1,0,0); 
    } else if (cascadeIdx == 1) {
        cascadeColor = vec3(0,1,0); 
    } else if (cascadeIdx == 2) {
        cascadeColor = vec3(0,0,1); 
    };
    
    colorOut = 0.5*cascadeColor + 0.5*colorOut;
}



vec3 computeViewPositionFromDepth(in vec2 texCoord, in float depth, in mat4 inverseMatrix) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depth * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = inverseMatrix * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};


void main()
{   
	vec2 texCoord = fs_in.tex_coords;

	vec3 albedo = texture(gBuffer.albedoMap, texCoord).rgb;
	
	vec3 aoMetalRoughness = texture(gBuffer.aoMetalRoughnessMap, texCoord).rgb;
	float ao = aoMetalRoughness.r;
	float metallic = aoMetalRoughness.g;
    //metallic = 0;
	float roughness = aoMetalRoughness.b;
    //roughness = 0.3;
	
	vec3 normalEye = normalize(texture(gBuffer.normalEyeMap, texCoord).rgb);
    
    //if (length(normalEye) < 0.01) {
        //discard;
    //}
	
    float depth = texture(gBuffer.normalizedViewSpaceZMap, texCoord).r;
    //float viewSpaceZ = denormalizeViewSpaceZ(normalizedViewSpaceZ, nearFarPlane.x, nearFarPlane.y);
    //vec3 positionEye = getViewPositionFromNormalizedZ(fs_in.tex_coords, viewSpaceZ, inverseProjMatrix_GPass);
    vec3 positionEye = computeViewPositionFromDepth(texCoord, depth, inverseProjMatrix_GPass);
    
    vec3 colorOut;
    vec3 luminanceOut;
    calcLighting(ao, 
                albedo, 
                metallic, 
                normalEye, 
                roughness, 
                positionEye,
                texCoord,
                colorOut,
                luminanceOut);
        
    FragColor = vec4(colorOut, 1.0);
    LuminanceColor = vec4(luminanceOut, FragColor.a);
}