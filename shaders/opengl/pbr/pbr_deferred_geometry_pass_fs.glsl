#version 420 core

layout(location = 0) out vec3 albedo;
layout(location = 1) out vec3 aoMetallRoughness;
layout(location = 2) out vec4 normalEye; // we use 10_10_10_2, so it will be a vec4
layout(location = 3) out float normalizedViewSpaceZ;
layout(location = 4) out vec2 motion;

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
    normalEye = vec4(getEncodedNormalEye(), 0);
	
	// position
	//positionEye = fs_in.fragment_position_eye.xyz;
    
    //normalizedViewSpaceZ = normalizeViewSpaceZ(fs_in.viewSpaceZ, nearFarPlane.x, nearFarPlane.y);
    normalizedViewSpaceZ = gl_FragCoord.z;
    
    
    
    
    /*motion = (fs_in.position_ndc.xy/fs_in.position_ndc.w - 
            fs_in.position_ndc_previous.xy / fs_in.position_ndc_previous.w)/2.f;*/
    motion = (fs_in.position_ndc.xy/fs_in.position_ndc.w - 
            fs_in.position_ndc_previous.xy / fs_in.position_ndc_previous.w);
            //motion = vec2(1.0);
}