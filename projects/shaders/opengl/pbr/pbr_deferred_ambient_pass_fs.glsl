#version 460 core

#ifndef USE_CONE_TRACING
#define USE_CONE_TRACING 1
#endif

#ifndef PBR_ALBEDO_BINDINPOINT
#define PBR_ALBEDO_BINDINPOINT 0
#endif

#ifndef PBR_AO_METAL_ROUGHNESS_BINDINPOINT
#define PBR_AO_METAL_ROUGHNESS_BINDINPOINT 1
#endif

#ifndef PBR_NORMAL_BINDINPOINT
#define PBR_NORMAL_BINDINPOINT 2
#endif

#ifndef PBR_DEPTH_BINDINPOINT
#define PBR_DEPTH_BINDINPOINT 3
#endif

#ifndef PBR_EMISSION_OBJECT_MATERIAL_ID_BINDINPOINT
#define PBR_EMISSION_OBJECT_MATERIAL_ID_BINDINPOINT 4
#endif

#include "pbr/pbr_common_lighting_fs.glsl"

layout(location = 0) out vec4 irradianceOut;
layout(location = 1) out vec4 ambientReflectionOut;

in VS_OUT {
    vec2 texCoord;
} fs_in;


struct GBuffer {
    layout(binding = PBR_ALBEDO_BINDINPOINT)                sampler2D albedoMap;
    layout(binding = PBR_AO_METAL_ROUGHNESS_BINDINPOINT)	sampler2D aoMetalRoughnessMap;
    layout(binding = PBR_NORMAL_BINDINPOINT)	            sampler2D normalEyeMap;
    layout(binding = PBR_DEPTH_BINDINPOINT)                 sampler2D depthMap;
    layout(binding = PBR_EMISSION_OBJECT_MATERIAL_ID_BINDINPOINT) sampler2D emissionObjectMaterialIDMap;
};

uniform GBuffer gBuffer;

vec3 getNormal() {

    vec3 a1 = normalize(2.0 * textureOffset(gBuffer.normalEyeMap, fs_in.texCoord, ivec2(0, 0)).rgb - 1.0);
    vec3 a2 = normalize(2.0 * textureOffset(gBuffer.normalEyeMap, fs_in.texCoord, ivec2(0, 1)).rgb - 1.0);
    vec3 a3 = normalize(2.0 * textureOffset(gBuffer.normalEyeMap, fs_in.texCoord, ivec2(1, 0)).rgb - 1.0);
    vec3 a4 = normalize(2.0 * textureOffset(gBuffer.normalEyeMap, fs_in.texCoord, ivec2(1, 1)).rgb - 1.0);
    
    vec3 a5 = normalize(2.0 * textureOffset(gBuffer.normalEyeMap, fs_in.texCoord, ivec2(0, -1)).rgb - 1.0);
    vec3 a6 = normalize(2.0 * textureOffset(gBuffer.normalEyeMap, fs_in.texCoord, ivec2(-1, 0)).rgb - 1.0);
    vec3 a7 = normalize(2.0 * textureOffset(gBuffer.normalEyeMap, fs_in.texCoord, ivec2(-1, -1)).rgb - 1.0);
    vec3 a8 = normalize(2.0 * textureOffset(gBuffer.normalEyeMap, fs_in.texCoord, ivec2(1, -1)).rgb - 1.0);
    vec3 a9 = normalize(2.0 * textureOffset(gBuffer.normalEyeMap, fs_in.texCoord, ivec2(-1, 1)).rgb - 1.0);
    
    return normalize((a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9));
}

vec3 getAlbedo() {

    vec3 a1 = textureOffset(gBuffer.albedoMap, fs_in.texCoord, ivec2(0, 0)).rgb;
    vec3 a2 = textureOffset(gBuffer.albedoMap, fs_in.texCoord, ivec2(0, 1)).rgb;
    vec3 a3 = textureOffset(gBuffer.albedoMap, fs_in.texCoord, ivec2(1, 0)).rgb;
    vec3 a4 = textureOffset(gBuffer.albedoMap, fs_in.texCoord, ivec2(1, 1)).rgb;
    
    vec3 a5 = textureOffset(gBuffer.albedoMap, fs_in.texCoord, ivec2(0, -1)).rgb;
    vec3 a6 = textureOffset(gBuffer.albedoMap, fs_in.texCoord, ivec2(-1, 0)).rgb;
    vec3 a7 = textureOffset(gBuffer.albedoMap, fs_in.texCoord, ivec2(-1, -1)).rgb;
    vec3 a8 = textureOffset(gBuffer.albedoMap, fs_in.texCoord, ivec2(1, -1)).rgb;
    vec3 a9 = textureOffset(gBuffer.albedoMap, fs_in.texCoord, ivec2(-1, 1)).rgb;
    
    return (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9) / 9.0;
}

vec3 getAoMetalRoughness() {

    vec3 a1 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(0, 0)).rgb;
    vec3 a2 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(0, 1)).rgb;
    vec3 a3 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(1, 0)).rgb;
    vec3 a4 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(1, 1)).rgb;
    
    vec3 a5 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(0, -1)).rgb;
    vec3 a6 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(-1, 0)).rgb;
    vec3 a7 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(-1, -1)).rgb;
    vec3 a8 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(1, -1)).rgb;
    vec3 a9 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(-1, 1)).rgb;
    
    return (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9) / 9.0;
}

float getDepth() {

    float a1 = textureOffset(gBuffer.depthMap, fs_in.texCoord, ivec2(0, 0)).r;
    float a2 = textureOffset(gBuffer.depthMap, fs_in.texCoord, ivec2(0, 1)).r;
    float a3 = textureOffset(gBuffer.depthMap, fs_in.texCoord, ivec2(1, 0)).r;
    float a4 = textureOffset(gBuffer.depthMap, fs_in.texCoord, ivec2(1, 1)).r;
    
    float a5 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(0, -1)).r;
    float a6 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(-1, 0)).r;
    float a7 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(-1, -1)).r;
    float a8 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(1, -1)).r;
    float a9 = textureOffset(gBuffer.aoMetalRoughnessMap, fs_in.texCoord, ivec2(-1, 1)).r;
    
    return a1;
    //return max(max(max(a1, a2), a3), a4);
    return (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9) / 9.0;
}


void main()
{
    vec2 texCoords = fs_in.texCoord;

	const vec3 albedo = getAlbedo();//texture(gBuffer.albedoMap, fs_in.texCoords).rgb;
	const vec3 aoMetalRoughness = getAoMetalRoughness();//texture(gBuffer.aoMetalRoughnessMap, fs_in.texCoords).rgb;
	const float ao = aoMetalRoughness.r;
	const float metallic = aoMetalRoughness.g;
	float roughness = aoMetalRoughness.b;
	
	const vec3 normalEye = getNormal();//normalize(2.0 * texture(gBuffer.normalEyeMap, fs_in.texCoords).rgb - 1.0);
	
    const float depth = getDepth();//texture(gBuffer.depthMap, fs_in.texCoords).r;
	
	if (depth == 0.0) discard;
	
    vec3 positionEye = reconstructPositionFromDepth(constants.invProjectionGPass, fs_in.texCoord, depth);
	
	//vec3 viewEye = normalize(-positionEye);
    vec3 viewWorld = normalize(vec3(constants.invViewGPass * vec4(-positionEye, 0.0f)));
    vec3 normalWorld = normalize(vec3(1.0, 1.0, 1.0) * vec3(constants.invViewGPass * vec4(normalEye, 0.0f)));
    vec3 positionWorld = vec3(constants.invViewGPass * vec4(positionEye, 1.0f));
	
	float distToCamera = length(positionEye);
	float blendFactor = clamp((distToCamera - 2.0) * 0.1, 0.0, 0.4);
	roughness = mix(roughness, 1.0, blendFactor);
	blendFactor = clamp((distToCamera - 20.0) * 0.1, 0.0, 1.0);
	roughness = mix(roughness, 1.0, blendFactor);
	
	uint objectMaterialID = uint(texture(gBuffer.emissionObjectMaterialIDMap, fs_in.texCoord).a);
	
	PerObjectMaterialData objectMaterialData = materials[objectMaterialID];
	
	int diffuseReflectionArrayIndex = objectMaterialData.probes.y;
	int specularReflectionArrayIndex = objectMaterialData.probes.z; 
		
    irradianceOut = pbrIrradiance(normalWorld, positionWorld, diffuseReflectionArrayIndex);
    ambientReflectionOut = pbrAmbientReflection(normalWorld, roughness, metallic, albedo, ao, positionWorld, viewWorld, specularReflectionArrayIndex);
   
}