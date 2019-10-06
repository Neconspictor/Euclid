#version 430 core

layout(location = 0) out vec4 FragColor;

in VS_OUT {	
	vec2 tex_coords;
} fs_in;

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


void main()
{
	const vec3 albedo = texture(gBuffer.albedoMap, fs_in.tex_coords).rgb;
	const vec3 aoMetalRoughness = texture(gBuffer.aoMetalRoughnessMap, fs_in.tex_coords).rgb;
	const float ao = aoMetalRoughness.r;
	const float metallic = aoMetalRoughness.g;
	const float roughness = aoMetalRoughness.b;
	
	const vec3 normalEye = normalize(2.0 * texture(gBuffer.normalEyeMap, fs_in.tex_coords).rgb - 1.0);
	
    const float depth = texture(gBuffer.depthMap, fs_in.tex_coords).r;
    vec3 positionEye = computeViewPositionFromDepth(fs_in.tex_coords, depth);
    
    FragColor = vec4(calcAmbientLighting(normalEye, positionEye, ao, albedo, metallic, roughness), 1.0);
}