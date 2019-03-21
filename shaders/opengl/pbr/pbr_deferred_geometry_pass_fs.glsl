#version 420 core

layout(location = 0) out vec3 albedo;
layout(location = 1) out vec3 aoMetallRoughness;
layout(location = 2) out vec3 normalEye;
layout(location = 3) out float normalizedViewSpaceZ;

#include "pbr/pbr_common_geometry_fs.glsl"


void main()
{    		
	// albedo color
	albedo = texture(material.albedoMap, fs_in.tex_coords).rgb;
	
	// ambient occlusion, metallic, roughness
	aoMetallRoughness.r = texture(material.aoMap, fs_in.tex_coords).r;
	aoMetallRoughness.g = texture(material.metallicMap, fs_in.tex_coords).r;
	aoMetallRoughness.b = texture(material.roughnessMap, fs_in.tex_coords).r;
	
	//normal
    normalEye = getNormalEye();
	
	// position
	//positionEye = fs_in.fragment_position_eye.xyz;
    
    //normalizedViewSpaceZ = normalizeViewSpaceZ(fs_in.viewSpaceZ, nearFarPlane.x, nearFarPlane.y);
    normalizedViewSpaceZ = gl_FragCoord.z;
}