#version 460 core

#include "pbr/pbr_common_lighting_fs.glsl"
#include "pbr/pbr_common_geometry_fs.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;

void main()
{    		
	//position
    const vec3 positionEye = fs_in.fragment_position_eye.rgb;
    //positionEye = vec3(0.0);
    
    // albedo color
	const vec4 albedo = texture(material.albedoMap, fs_in.tex_coords).rgba;
	
	// ambient occlusion, metallic, roughness
	const float ao = texture(material.aoMap, fs_in.tex_coords).r;
	const float metallic = texture(material.metallicMap, fs_in.tex_coords).r;
	const float roughness = texture(material.roughnessMap, fs_in.tex_coords).r;
	
	//normal
    const vec3 normalEye = 2.0 * getEncodedNormalEye() - 1.0;
    
    // view direction
	const vec3 viewEye = normalize(-positionEye);
    
    // metallic workflow
    const vec3 F0 = vec3(0.4);
                
    const vec3 directLighting = pbrDirectLight(viewEye, normalEye, dirLight.directionEye.xyz, roughness, F0, metallic, albedo.rgb);            
        
    //FragColor = vec3(0.4 * albedo.rgb + 0.6 * directLighting);
    FragColor = vec4((0.4 * albedo.rgb + 0.6 * directLighting), albedo.a); //albedo.a
    //FragColor = vec4(1.0f);

    
    LuminanceColor = vec4(directLighting * 0.5, FragColor.a);
    //LuminanceColor = vec4(0.0, 0.0, 0.0, 1.0);
}