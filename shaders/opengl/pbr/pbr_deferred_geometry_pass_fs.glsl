#version 420 core

#include "pbr/viewspaceNormalization.glsl"

layout(location = 0) out vec3 albedo;
layout(location = 1) out vec3 aoMetallRoughness;
layout(location = 2) out vec3 normalEye;
layout(location = 3) out float normalizedViewSpaceZ;

const float PI = 3.14159265359;

struct Material {
    layout(binding = 0) sampler2D albedoMap;
	layout(binding = 1) sampler2D aoMap;
	layout(binding = 2) sampler2D metallicMap;
	layout(binding = 3) sampler2D normalMap;
	layout(binding = 4) sampler2D roughnessMap;
};


in VS_OUT {	
	vec4 fragment_position_eye;
    float viewSpaceZ;
	vec2 tex_coords;
	mat3 TBN_eye_directions; // used to transform the normal vector from tangent to eye space.
						  //  This matrix mustn't be used with positions!!!
} fs_in;


uniform Material material;

// Viewspace z values of the near and far plane
// Note: near and far mustn't be equal (Divide by zero!)
uniform vec2 nearFarPlane;


void main()
{    		
	
	// albedo color
	albedo = texture(material.albedoMap, fs_in.tex_coords).rgb;
	
	// ambient occlusion, metallic, roughness
	aoMetallRoughness.r = texture(material.aoMap, fs_in.tex_coords).r;
	aoMetallRoughness.g = texture(material.metallicMap, fs_in.tex_coords).r;
	aoMetallRoughness.b = texture(material.roughnessMap, fs_in.tex_coords).r;
	
	//normal
	float factor = 255/128.0f;	// is better than 2.0f for precision reasons!
	vec3 N = (texture(material.normalMap, fs_in.tex_coords).xyz * factor) - 1.0;
	//N = vec3(0,0,1);
	
	//mat3 tbnTangentToEye = inverse(fs_in.TBN_eye_directions);
	mat3 tbnTangentToEye = fs_in.TBN_eye_directions;
	
	normalEye = normalize(tbnTangentToEye * N);
	//normalEye = texture(material.normalMap, fs_in.tex_coords).xyz;
	//normalEye = normalEye*0.5 + 0.5;
	
	// position
	//positionEye = fs_in.fragment_position_eye.xyz;
    
    normalizedViewSpaceZ = normalizeViewSpaceZ(fs_in.viewSpaceZ, nearFarPlane.x, nearFarPlane.y);
}