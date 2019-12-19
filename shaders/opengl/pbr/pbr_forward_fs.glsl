#version 460 core

#include "pbr/pbr_common_geometry_fs.glsl"
#include "pbr/pbr_common_lighting_fs.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;

void main()
{    		
	//position
    vec3 positionEye = fs_in.fragment_position_eye.rgb;
    //positionEye = vec3(0.0);
    
    // albedo color
	vec4 albedo = texture(material.albedoMap, fs_in.tex_coords).rgba;
	
	// ambient occlusion, metallic, roughness
	float ao = texture(material.aoMap, fs_in.tex_coords).r;
	float metallic = texture(material.metallicMap, fs_in.tex_coords).r;
	float roughness = texture(material.roughnessMap, fs_in.tex_coords).r;
	
	//normal
    vec3 normalEye = 2.0 * getEncodedNormalEye() - 1.0;
    
    //vec2 texCoords = fs_in.tex_coords;
    
    
    vec3 colorOut;
    vec3 luminanceOut;
    calcLighting(ao, 
                albedo.rgb, 
                metallic, 
                normalEye, 
                roughness, 
                positionEye,
                colorOut,
                luminanceOut);
        
    FragColor = vec4(colorOut, albedo.a);
    //LuminanceColor = vec4(luminanceOut, FragColor.a);
    LuminanceColor = vec4(0.0, 0.0, 0.0, 1.0);
}