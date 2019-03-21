#version 430

#include "pbr/pbr_common_lighting_fs.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;

in VS_OUT {	
	vec2 tex_coords;
} fs_in;

struct GBuffer {
    sampler2D albedoMap;
	sampler2D aoMetalRoughnessMap;
	sampler2D normalEyeMap;
    sampler2D normalizedViewSpaceZMap;
};

uniform GBuffer gBuffer;

uniform mat4 inverseViewMatrix_GPass; // the inverse view from the geometry pass!
uniform mat4 inverseProjMatrix_GPass;
uniform vec2 nearFarPlane;


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
	vec3 albedo = texture(gBuffer.albedoMap, fs_in.tex_coords).rgb;
	
	vec3 aoMetalRoughness = texture(gBuffer.aoMetalRoughnessMap, fs_in.tex_coords).rgb;
	float ao = aoMetalRoughness.r;
	float metallic = aoMetalRoughness.g;
    //metallic = 0;
	float roughness = aoMetalRoughness.b;
    //roughness = 0.3;
	
	vec3 normalEye = normalize(texture(gBuffer.normalEyeMap, fs_in.tex_coords).rgb);
    
    if (length(normalEye) < 0.01) {
        discard;
    }
	
    float depth = texture(gBuffer.normalizedViewSpaceZMap, fs_in.tex_coords).r;
    //float viewSpaceZ = denormalizeViewSpaceZ(normalizedViewSpaceZ, nearFarPlane.x, nearFarPlane.y);
    //vec3 positionEye = getViewPositionFromNormalizedZ(fs_in.tex_coords, viewSpaceZ, inverseProjMatrix_GPass);
    vec3 positionEye = computeViewPositionFromDepth(fs_in.tex_coords, depth, inverseProjMatrix_GPass);
	
	float ambientOcclusion = texture(ssaoMap, fs_in.tex_coords).r;
	
	// calculate per-light radiance
	vec3 viewEye = normalize(-positionEye);
    
    
    vec3 viewWorld = vec3(inverseViewMatrix_GPass * vec4(viewEye, 0.0f));
    vec3 normalWorld = vec3(inverseViewMatrix_GPass * vec4(normalEye, 0.0f));
	
    vec3 reflectionDirWorld = reflect(-viewWorld, normalWorld);
    
    vec3 colorOut;
    vec3 luminanceOut;
    calcLighting(ao, 
                albedo, 
                metallic, 
                normalEye, 
                normalWorld, 
                roughness, 
                positionEye,
                viewEye, 
                reflectionDirWorld,
                fs_in.tex_coords,
                colorOut,
                luminanceOut);
        
    FragColor = vec4(colorOut, 1.0);
    LuminanceColor = vec4(luminanceOut, FragColor.a);
}