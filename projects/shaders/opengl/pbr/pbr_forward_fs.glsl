#version 460 core

#include "pbr/pbr_common_lighting_fs.glsl"
#include "pbr/pbr_common_geometry_fs.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;
layout(location = 2) out vec2 motion;


void main()
{    		
	//position
    vec3 positionEye = fs_in.fragment_position_eye.rgb;
	vec3 positionWorld = fs_in.fragment_position_world.xyz;
    //positionEye = vec3(0.0);
    
    // albedo color
	vec4 albedo = texture(material.albedoMap, fs_in.tex_coords).rgba;
	
	// ambient occlusion, metallic, roughness
	float ao = texture(material.aoMap, fs_in.tex_coords).r;
	float metallic = texture(material.metallicMap, fs_in.tex_coords).b;
	float roughness = texture(material.roughnessMap, fs_in.tex_coords).g;
	
	PerObjectMaterialData objectMaterialData = materials[objectData.perObjectMaterialID];
	
	vec3 emission = intBitsToFloat(objectMaterialData.probesEmission.z) * texture(material.emissionMap, fs_in.tex_coords).rgb;
	
	
	/*float distToCamera = length(fs_in.camera_position_world.xyz - fs_in.fragment_position_world.xyz);
	float blendFactor = clamp((distToCamera - 2.0) * 0.1, 0.0, 0.4);
	roughness = mix(roughness, 1.0, blendFactor);
	blendFactor = clamp((distToCamera - 20.0) * 0.1, 0.0, 1.0);
	roughness = mix(roughness, 1.0, blendFactor);*/
	//roughness = 1.0;
	//normal
    const vec3 normalEye = getNormalEye();
	const vec3 normalWorld = normalize(vec3(constants.invViewGPass * vec4(normalEye, 0.0)));
    
	
	
	// reflection direction
    
	
	// View from fragment to camera
	vec3 viewWorld = normalize(fs_in.camera_position_world.xyz - positionWorld);//normalize(inverseViewMatrix[3].xyz - positionWorld); // Note inverse view -> negative camera position
	//vec3 viewEye = normalize(-positionEye);
    //vec3 viewWorld = normalize(vec3(inverseViewMatrix * vec4(viewEye, 0.0f)));
    //vec3 normalWorld = normalize(vec3(1.0, 1.0, 1.0) * vec3(inverseViewMatrix * vec4(normalEye, 0.0f)));
	
	
    //vec2 texCoords = fs_in.tex_coords;
    
    
    vec3 colorOut;
    vec3 luminanceOut;
    calcCompoundLighting(ao, 
                albedo.rgb, 
                metallic, 
                normalWorld, 
                roughness, 
                positionWorld,
				positionEye,
				viewWorld,
                colorOut,
                luminanceOut);
        
    FragColor = vec4(colorOut, albedo.a); //albedo.a
	
	//FragColor = vec4(0,1,0,1);
	
    LuminanceColor = vec4(luminanceOut + emission, FragColor.a);
	
	motion = (fs_in.position_ndc.xy / fs_in.position_ndc.w - fs_in.position_ndc_previous.xy / fs_in.position_ndc_previous.w);
    //LuminanceColor = vec4(0.0, 0.0, 0.0, 1.0);
}