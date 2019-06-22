#version 430 core

#include "pbr/pbr_common_lighting_fs.glsl"

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

//uniform mat4 inverseViewMatrix_GPass; // the inverse view from the geometry pass!
uniform mat4 inverseProjMatrix_GPass;
uniform vec2 nearFarPlane;


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
	//const vec2 texCoord = fs_in.tex_coords;

	const vec3 albedo = texture(gBuffer.albedoMap, fs_in.tex_coords).rgb;
	
	const vec3 aoMetalRoughness = texture(gBuffer.aoMetalRoughnessMap, fs_in.tex_coords).rgb;
	const float ao = aoMetalRoughness.r;
	float metallic = aoMetalRoughness.g;
    //metallic = 0;
	float roughness = aoMetalRoughness.b;
    //roughness = 1;
	
	const vec3 normalEye = normalize(texture(gBuffer.normalEyeMap, fs_in.tex_coords).rgb);
    
    //if (length(normalEye) < 0.01) {
        //discard;
    //}
	
    const float depth = texture(gBuffer.normalizedViewSpaceZMap, fs_in.tex_coords).r;
    //float viewSpaceZ = denormalizeViewSpaceZ(normalizedViewSpaceZ, nearFarPlane.x, nearFarPlane.y);
    //vec3 positionEye = getViewPositionFromNormalizedZ(fs_in.tex_coords, viewSpaceZ, inverseProjMatrix_GPass);
    const vec3 positionEye = computeViewPositionFromDepth(fs_in.tex_coords, depth);
    
    vec3 colorOut;
    vec3 luminanceOut;
    calcLighting(ao, 
                albedo, 
                metallic, 
                normalEye, 
                roughness, 
                positionEye,
                //fs_in.tex_coords,
                colorOut,
                luminanceOut);
        
    FragColor = vec4(colorOut, 1.0);
    LuminanceColor = vec4(luminanceOut, FragColor.a);
}