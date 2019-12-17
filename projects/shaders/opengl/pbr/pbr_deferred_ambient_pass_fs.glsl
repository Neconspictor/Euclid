#version 460 core

#ifndef USE_CONE_TRACING
#define USE_CONE_TRACING
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

#ifndef PBR_EMISSION_BINDINPOINT
#define PBR_EMISSION_BINDINPOINT 4
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
    //layout(binding = PBR_EMISSION_BINDINPOINT)            sampler2D emissionMap;
};

uniform GBuffer gBuffer;

//uniform mat4 inverseViewMatrix_GPass; // the inverse view from the geometry pass!
uniform mat4 inverseProjMatrix_GPass;


vec3 computeViewPositionFromDepth(in vec2 texCoord, in float depth) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depth * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = inverseProjMatrix_GPass * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};

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
	const float roughness = aoMetalRoughness.b;
	
	const vec3 normalEye = getNormal();//normalize(2.0 * texture(gBuffer.normalEyeMap, fs_in.texCoords).rgb - 1.0);
	
    const float depth = getDepth();//texture(gBuffer.depthMap, fs_in.texCoords).r;
    vec3 positionEye = computeViewPositionFromDepth(fs_in.texCoord, depth);
    
    calcAmbientLighting3(normalEye, positionEye, ao, albedo, metallic, roughness, irradianceOut, ambientReflectionOut);
}