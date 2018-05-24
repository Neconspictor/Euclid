#version 400 core

layout(location = 0) out vec3 albedo;
layout(location = 1) out float ao;
layout(location = 2) out float metallic;
layout(location = 3) out vec3 normalEye;
layout(location = 4) out vec3 positionEye;
layout(location = 5) out float roughness;

const float PI = 3.14159265359;

struct Material {
    sampler2D albedoMap;
	sampler2D aoMap;
	sampler2D metallicMap;
	sampler2D normalMap;
	sampler2D roughnessMap;
	sampler2D shadowMap;
};


in VS_OUT {	
	vec4 fragment_position_eye;
	vec2 tex_coords;
	mat3 TBN_eye_directions; // used to transform the normal vector from tangent to eye space.
						  //  This matrix mustn't be used with positions!!!
} fs_in;


uniform Material material;


void main()
{    		
	
	// albedo color
	albedo = texture(material.albedoMap, fs_in.tex_coords).rgb;
	
	// ambient occlusion
	ao = texture(material.aoMap, fs_in.tex_coords).r;
	
	// metallic
	metallic = texture(material.metallicMap, fs_in.tex_coords).r;
	
	//normal
	float factor = 255/128.0f;	// is better than 2.0f for precision reasons!
	vec3 N = (texture(material.normalMap, fs_in.tex_coords).xyz * factor) - 1.0;
	normalEye = normalize(fs_in.TBN_eye_directions * N);
	//normalEye = texture(material.normalMap, fs_in.tex_coords).xyz;
	//normalEye = normalEye*0.5 + 0.5;
	
	// position
	positionEye = fs_in.fragment_position_eye.xyz;

	// roughness
	roughness = texture(material.roughnessMap, fs_in.tex_coords).r;
}