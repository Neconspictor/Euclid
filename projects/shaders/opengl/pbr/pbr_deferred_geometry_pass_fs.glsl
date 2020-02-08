#version 460 core

layout(location = 0) out vec3 albedo;
layout(location = 1) out vec3 aoMetallRoughness;
layout(location = 2) out vec4 normalEye; // we use 10_10_10_2, so it will be a vec4
layout(location = 3) out float depth;
layout(location = 4) out uint outPerObjectMaterialID;
layout(location = 5) out vec2 motion;


#include "pbr/pbr_common_geometry_fs.glsl"


void main()
{    		
	// albedo color
    vec4 albedoRaw = texture(material.albedoMap, fs_in.tex_coords);
    
    // stenciling
    if (albedoRaw.a < 0.9) {
        //discard;
    }
    
	albedo = albedoRaw.rgb;	
	// ambient occlusion, metallic, roughness
	aoMetallRoughness.r = texture(material.aoMap, fs_in.tex_coords).r;
	aoMetallRoughness.g = texture(material.metallicMap, fs_in.tex_coords).r;
	aoMetallRoughness.b = texture(material.roughnessMap, fs_in.tex_coords).r ;
    
	//normal
    normalEye = vec4(getEncodedNormalEye(), 0);
	
	// position
	//positionEye = fs_in.fragment_position_eye.xyz;
    
    //normalizedViewSpaceZ = normalizeViewSpaceZ(fs_in.viewSpaceZ, nearFarPlane.x, nearFarPlane.y);
    depth = gl_FragCoord.z;
    
    outPerObjectMaterialID = objectData.perObjectMaterialID;	//float(fs_in2.perObjectMaterialID);
    
    
    /*motion = (fs_in.position_ndc.xy/fs_in.position_ndc.w - 
            fs_in.position_ndc_previous.xy / fs_in.position_ndc_previous.w)/2.f;*/
    motion = (fs_in.position_ndc.xy / fs_in.position_ndc.w - fs_in.position_ndc_previous.xy / fs_in.position_ndc_previous.w);
            //motion = vec2(1.0);
}