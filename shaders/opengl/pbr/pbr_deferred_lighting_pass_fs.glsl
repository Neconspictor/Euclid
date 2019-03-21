#version 430

#include "shadow/cascaded_shadow.glsl"
#include "pbr/viewspaceNormalization.glsl"
#include "pbr/pbr_common_fs.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminaceColor;

in VS_OUT {	
	vec2 tex_coords;
} fs_in;

struct GBuffer {
    sampler2D albedoMap;
	sampler2D aoMetalRoughnessMap;
	sampler2D normalEyeMap;
    sampler2D normalizedViewSpaceZMap;
};

uniform GBuffer gBuffer;

uniform mat4 inverseViewMatrix_GPass; // the inverse view from the geometry pass!


// Cascaded shadow mapping
layout(std140,binding=0) buffer CascadeBuffer { //buffer uniform
	/*mat4 inverseViewMatrix;
	mat4 lightViewProjectionMatrices[CSM_NUM_CASCADES];
    vec4 scaleFactors[CSM_NUM_CASCADES];
	vec4 cascadedSplits[CSM_NUM_CASCADES];*/
    CascadeData cascadeData;
} csmData;

uniform sampler2DArray cascadedDepthMap;
uniform sampler2D ssaoMap;
uniform mat4 inverseProjMatrix_GPass;
uniform vec2 nearFarPlane;


vec3 computeViewPositionFromDepth(in vec2 texCoord, in float depth, in mat4 inverseMatrix) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depth * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = inverseMatrix * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};


void main()
{   
	vec3 albedo = texture(gBuffer.albedoMap, fs_in.tex_coords).rgb;
	
	vec3 aoMetalRoughness = texture(gBuffer.aoMetalRoughnessMap, fs_in.tex_coords).rgb;
	float ao = aoMetalRoughness.r;
	float metallic = aoMetalRoughness.g;
    //metallic = 0;
	float roughness = aoMetalRoughness.b;
    //roughness = 0.3;
	
	vec3 normalEye = normalize(texture(gBuffer.normalEyeMap, fs_in.tex_coords).rgb);
    
    if (length(normalEye) < 0.01) {
        discard;
    }
    
	/*float alpha = length(normalEye);
	if (alpha < 0.9) {
		discard;
	};*/
	
    float depth = texture(gBuffer.normalizedViewSpaceZMap, fs_in.tex_coords).r;
    //float viewSpaceZ = denormalizeViewSpaceZ(normalizedViewSpaceZ, nearFarPlane.x, nearFarPlane.y);
    //vec3 positionEye = getViewPositionFromNormalizedZ(fs_in.tex_coords, viewSpaceZ, inverseProjMatrix_GPass);
    vec3 positionEye = computeViewPositionFromDepth(fs_in.tex_coords, depth, inverseProjMatrix_GPass);
	
	float ambientOcclusion = texture(ssaoMap, fs_in.tex_coords).r;
	
	// calculate per-light radiance
	vec3 lightEye =  normalize(-dirLight.directionEye);
	//lightEye = normalize(vec3(view * (-1,-1,0,0)));
	//lightEye = normalize(vec3(1,1,1));
	//vec3 lightEye = normalize((viewGPass * vec4(-dirLight.directionEye, 0)).rgb);
	vec3 viewEye = normalize(-positionEye);
    
    
    vec3 viewWorld = vec3(inverseViewMatrix_GPass * vec4(viewEye, 0.0f));
    //viewWorld = vec3(0,0,1);
    vec3 normalWorld = vec3(inverseViewMatrix_GPass * vec4(normalEye, 0.0f));
    //normalWorld = vec3(0,1,0);
	
    vec3 reflectionDir = reflect(-viewWorld, normalWorld);
    
    CascadeData cascadeData = csmData.cascadeData;
    float fragmentLitProportion = cascadedShadow(-dirLight.directionEye, normalEye, positionEye.z, positionEye, cascadeData, cascadedDepthMap);
	
    vec3 directLighting;
    vec3 result = pbrModel(ao, 
		albedo, 
		metallic, 
		normalEye, 
        normalWorld,
		roughness, 
		viewEye,
		reflectionDir,
		fragmentLitProportion,
		ambientOcclusion,
        directLighting);

	
	FragColor = vec4(result, 1) ;//* scale;
    
    
    uint cascadeIdx = getCascadeIdx(positionEye.z, cascadeData);
    cascadeIdx = 10;
    
    vec4 cascadeColor = FragColor;
    
    if (cascadeIdx == 0) {
    
       cascadeColor = vec4(1,0,0,1); 
    
    } else if (cascadeIdx == 1) {
        cascadeColor = vec4(0,1,0,1); 
    } else if (cascadeIdx == 2) {
        cascadeColor = vec4(0,0,1,1); 
    };
    
    FragColor = 0.5*cascadeColor + 0.5*FragColor;
    LuminaceColor = vec4(directLighting.rgb, FragColor.a);
}