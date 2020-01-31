#version 460 core

#include "pbr/pbr_common_lighting_fs.glsl"
#include "pbr/pbr_common_geometry_fs.glsl"

layout(location = 0) out vec4 FragColor;
//layout(location = 1) out vec4 LuminanceColor;



void calcLighting(in float ao, 
             in vec3 albedo, 
             in float metallic, 
             in vec3 normalEye, 
             in float roughness,
             in vec3 positionEye,             
             //in vec2 texCoord,
             out vec3 colorOut,
             out vec3 luminanceOut) 
{
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	
	// reflection direction
    vec3 positionWorld = vec3(inverseViewMatrix * vec4(positionEye, 1.0f));
	
	// View from fragment to camera
	vec3 viewWorld = normalize(inverseViewMatrix[3].xyz - positionWorld); // Note inverse view -> negative camera position
	vec3 viewEye = normalize(-positionEye);
    //vec3 viewWorld = normalize(vec3(inverseViewMatrix * vec4(viewEye, 0.0f)));
    vec3 normalWorld = normalize(vec3(1.0, 1.0, 1.0) * vec3(inverseViewMatrix * vec4(normalEye, 0.0f)));
    vec3 reflectionDirWorld = normalize(reflect(-viewWorld, normalWorld));
    
    
    

    // reflectance equation
    vec3 Lo = pbrDirectLight(viewEye, normalEye, dirLight.directionEye, roughness, F0, metallic, albedo);
    
    vec3 ambient = pbrAmbientLight(normalWorld, roughness, F0, metallic, albedo, reflectionDirWorld, ao, positionWorld, viewWorld);
    
    float fragmentLitProportion = cascadedShadow(dirLight.directionEye, normalEye, positionEye.z, positionEye);
	
    vec3 color = ambient;// + albedo * 0.01 * ambientLightPower; //* ambientShadow; // ssaoAmbientOcclusion;
    float ambientShadow = clamp(fragmentLitProportion, 1.0 - shadowStrength, 1.0);
    color -= color*(1.0 - ambientShadow);
	
	// shadows affecting only direct light contribution
	//color += Lo * shadow;
    vec3 directLighting = fragmentLitProportion * Lo;
    
	color += directLighting;
    
    colorOut = color;
    luminanceOut = 0.01 * directLighting;
    
    return;
    
/*    //Debug
    uint cascadeIdx = getCascadeIdx(positionEye.z);
    //cascadeIdx = 10;
    
    vec3 cascadeColor = colorOut;
    
    if (cascadeIdx == 0) {
       cascadeColor = vec3(1,0,0); 
    } else if (cascadeIdx == 1) {
        cascadeColor = vec3(0,1,0); 
    } else if (cascadeIdx == 2) {
        cascadeColor = vec3(0,0,1); 
    } else if (cascadeIdx == 3) {
        cascadeColor = vec3(0,1,1);
    }
    
    cascadeColor = vec3(1,0,0);
    
    colorOut = mix(cascadeColor, color, 0.5);
	*/
}



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
    vec3 normalEye = getNormalEye();
    
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
	
	//FragColor = vec4(0,1,0,1);
	
    //LuminanceColor = vec4(luminanceOut, FragColor.a);
    //LuminanceColor = vec4(0.0, 0.0, 0.0, 1.0);
}