#version 430

#include "pbr/pbr_common_geometry_fs.glsl"
#include "pbr/pbr_common_lighting_fs.glsl"

in VS_OUT2 {
    vec4 fragment_position_eye;
} fs_in2;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;

void main()
{    		
	//position
    vec3 positionEye = fs_in2.fragment_position_eye.rgb;
    
    // albedo color
	vec3 albedo = texture(material.albedoMap, fs_in.tex_coords).rgb;
	
	// ambient occlusion, metallic, roughness
	float ao = texture(material.aoMap, fs_in.tex_coords).r;
	float metallic = texture(material.metallicMap, fs_in.tex_coords).r;
	float roughness = texture(material.roughnessMap, fs_in.tex_coords).r;
	
	//normal
    vec3 normalEye = getNormalEye();
    
    vec2 texCoords = fs_in.tex_coords;
    
    
    vec3 colorOut;
    vec3 luminanceOut;
    calcLighting(ao, 
                albedo, 
                metallic, 
                normalEye, 
                roughness, 
                positionEye,
                texCoords,
                colorOut,
                luminanceOut);
        
    FragColor = vec4(colorOut, 1.0);
    LuminanceColor = vec4(luminanceOut, FragColor.a);
}