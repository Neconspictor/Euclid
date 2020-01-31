#version 460 core

#ifndef USE_CONE_TRACING
#define USE_CONE_TRACING 1
#endif

#ifndef PBR_ALBEDO_BINDINPOINT
#define PBR_ALBEDO_BINDINPOINT 0
#endif

#ifndef PBR_AO_METAL_ROUGHNESS_BINDINPOINT
#define PBR_AO_METAL_ROUGHNESS_BINDINPOINT 1
#endif

#ifndef PBR_NORMAL_BINDINPOINT
#define PBR_NORMAL_BINDINPOINT 2
#endif

#ifndef PBR_DEPTH_BINDINPOINT
#define PBR_DEPTH_BINDINPOINT 3
#endif

#ifndef PBR_EMISSION_BINDINPOINT
#define PBR_EMISSION_BINDINPOINT 4
#endif

#include "pbr/pbr_common_lighting_fs.glsl"
#include "util/depth_util.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;



in VS_OUT {
    vec2 texCoord;
} fs_in;


struct GBuffer {
    layout(binding = PBR_ALBEDO_BINDINPOINT)                sampler2D albedoMap;
    layout(binding = PBR_AO_METAL_ROUGHNESS_BINDINPOINT)	sampler2D aoMetalRoughnessMap;
    layout(binding = PBR_NORMAL_BINDINPOINT)	            sampler2D normalEyeMap;
    layout(binding = PBR_DEPTH_BINDINPOINT)                 sampler2D depthMap;
    //layout(binding = PBR_EMISSION_BINDINPOINT)            sampler2D emissionMap;
};

uniform GBuffer gBuffer;

//uniform mat4 inverseViewMatrix_GPass; // the inverse view from the geometry pass!
uniform mat4 inverseProjMatrix_GPass;
uniform vec2 nearFarPlane;


void main()
{   
	//const vec2 texCoord = fs_in.texCoord;

	const vec3 albedo = texture(gBuffer.albedoMap, fs_in.texCoord).rgb;
	
	const vec3 aoMetalRoughness = texture(gBuffer.aoMetalRoughnessMap, fs_in.texCoord).rgb;
	const float ao = aoMetalRoughness.r;
	float metallic = aoMetalRoughness.g;
    //metallic = 0;
	float roughness = aoMetalRoughness.b;
	
	const vec3 normalEye = normalize(2.0 * texture(gBuffer.normalEyeMap, fs_in.texCoord).rgb - 1.0);
    
    //if (length(normalEye) < 0.01) {
        //discard;
    //}
	
    const float depth = texture(gBuffer.depthMap, fs_in.texCoord).r;
    //float viewSpaceZ = denormalizeViewSpaceZ(normalizedViewSpaceZ, nearFarPlane.x, nearFarPlane.y);
    //vec3 positionEye = getViewPositionFromNormalizedZ(fs_in.texCoord, viewSpaceZ, inverseProjMatrix_GPass);
    vec3 positionEye = reconstructPositionFromDepth(inverseProjMatrix_GPass, fs_in.texCoord, depth);
    //positionEye += normalEye;
    
    vec4 irradiance = texture(irradianceOutMap, fs_in.texCoord);
    const vec4 ambientReflection = texture(ambientReflectionOutMap, fs_in.texCoord);
	vec3 irradianceResolved = irradiance.a * irradiance.rgb;
 
    vec3 ambient = calcAmbientLighting2(normalEye, positionEye, ao, albedo, metallic, roughness, irradianceResolved, ambientReflection.rgb);
	
	//ambient += mix(vec3(0.0), albedo * 0.025, 1 - irradiance.a );
    
    
    vec3 colorOut;
    vec3 luminanceOut;
    calcDirectLighting(ao, 
                albedo, 
                metallic, 
                normalEye, 
                roughness, 
                positionEye,
                colorOut,
                luminanceOut);
        
    FragColor = vec4(colorOut + ambient, 1.0);
    
    //Debug
   /* uint cascadeIdx = getCascadeIdx(positionEye.z);
    //cascadeIdx = 10;
    
    vec4 cascadeColor = FragColor;
    
    if (cascadeIdx == 0) {
       cascadeColor *= vec4(1,0,0,1); 
    } else if (cascadeIdx == 1) {
        cascadeColor *= vec4(0,1,0,1); 
    } else if (cascadeIdx == 2) {
        cascadeColor *= vec4(0,0,1,1); 
    } else if (cascadeIdx == 3) {
        cascadeColor *= vec4(0,1,1,1);
    }
    
    FragColor = mix(cascadeColor, FragColor, 0.8);*/
    
    
    
    LuminanceColor = vec4(luminanceOut, FragColor.a);
}