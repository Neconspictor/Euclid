#version 430

#include "shadow/cascaded_shadow.glsl"
#include "pbr/viewspaceNormalization.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminaceColor;

const float PI = 3.14159265359;

struct GBuffer {
    sampler2D albedoMap;
	sampler2D aoMetalRoughnessMap;
	sampler2D normalEyeMap;
    sampler2D normalizedViewSpaceZMap;
};


struct DirLight {
    vec3 directionEye;
    vec3 color;
    float power;
};


in VS_OUT {	
	vec2 tex_coords;
} fs_in;

uniform DirLight dirLight;

uniform float ambientLightPower;

uniform float shadowStrength;

uniform GBuffer gBuffer;
//uniform sampler2D shadowMap;
uniform sampler2D ssaoMap;

uniform mat4 inverseViewMatrix_GPass; // the inverse view from the geometry pass!
//uniform mat4 eyeToLight;
//uniform mat4 viewGPass;


// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;


// Cascaded shadow mapping
layout(std140,binding=0) buffer CascadeBuffer { //buffer uniform
	/*mat4 inverseViewMatrix;
	mat4 lightViewProjectionMatrices[CSM_NUM_CASCADES];
    vec4 scaleFactors[CSM_NUM_CASCADES];
	vec4 cascadedSplits[CSM_NUM_CASCADES];*/
    CascadeData cascadeData;
} csmData;

uniform sampler2DArray cascadedDepthMap;

uniform mat4 inverseProjMatrix_GPass;

uniform vec2 nearFarPlane;


vec3 pbrModel(float ao,
		vec3 albedo,
		float metallic, 
		vec3 normal,
		float roughness,
		vec3 viewDir,
		vec3 lightDir,
		vec3 reflectionDir,
		float shadow,
		float ssaoAmbientOcclusion,
        out vec3 directLighting);


vec3 pbrDirectLight(vec3 V, vec3 N, vec3 L, float roughness, vec3 F0, float metallic, vec3 albedo);
vec3 pbrAmbientLight(vec3 V, vec3 N, float roughness, vec3 F0, float metallic, vec3 albedo, vec3 R, float ao);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);


float cascadedShadow(vec3 lightDirection, vec3 normal, float depthViewSpace,vec3 viewPosition);

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
	/*float alpha = length(normalEye);
	if (alpha < 0.9) {
		discard;
	};*/
	
	
    float normalizedViewSpaceZ = texture(gBuffer.normalizedViewSpaceZMap, fs_in.tex_coords).r;
    float viewSpaceZ = denormalizeViewSpaceZ(normalizedViewSpaceZ, nearFarPlane.x, nearFarPlane.y);
    vec3 positionEye = getViewPositionFromNormalizedZ(fs_in.tex_coords, viewSpaceZ, inverseProjMatrix_GPass);
	
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
    //reflectionDir  = vec3(0,0,0);
	//vec3 reflectionDir = reflect(-viewEye, normalEye);
	//reflectionDir = vec3(inverseViewMatrix_GPass * vec4(reflectionDir, 0.0f)); // reflectionDir needs to be in world space
	
	//directional shadow calculation
	//vec4 positionLight = eyeToLight * vec4(positionEye.rgb, 1.0);
	
    //float shadow = shadowCalculation(shadowMap, lightEye, normalEye, positionLight);
	//cascadedShadow(vec3 lightDirection, vec3 normal, float depthViewSpace,vec3 viewPosition)
	//float shadow = cascadedShadow(-dirLight.directionEye, normalEye, positionEye.z, positionEye);
	
    
    
    
    //CascadeData cascadeData;
    /*cascadeData.inverseViewMatrix = csmData.cascadeData.inverseViewMatrix;
    
    for (uint i = 0; i < CSM_NUM_CASCADES; ++i) {
        cascadeData.lightViewProjectionMatrices[i] = csmData.cascadeData.lightViewProjectionMatrices[i];
        cascadeData.scaleFactors[i] = csmData.cascadeData.scaleFactors[i];
        cascadeData.cascadedSplits[i] = csmData.cascadeData.cascadedSplits[i];
    }*/
    
    CascadeData cascadeData = csmData.cascadeData;
    
    float fragmentLitProportion = cascadedShadow(-dirLight.directionEye, normalEye, positionEye.z, positionEye, cascadeData, cascadedDepthMap);
	//float fragmentLitProportion = csmData.value;
	
    vec3 directLighting;
    vec3 result = pbrModel(ao, 
		albedo, 
		metallic, 
		normalEye, 
		roughness, 
		viewEye, 
		lightEye, 
		reflectionDir,
		fragmentLitProportion,
		ambientOcclusion,
        directLighting);

	
	//alpha = clamp(alpha, 0, 1);
    
    //vec4 scale = cascadeData.lightViewProjectionMatrices[2] * vec4(1.0,0,1.0,1.0);
	
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
    
    //FragColor = vec4(0.8, 0.8, 0.8, 1);
    
	//vec2 windowSize = gl_FragCoord.xy / textureSize(gBuffer.positionEyeMap, 0).xy;
	//FragColor = vec4(windowSize, 1, 1);
}

vec3 pbrModel(float ao,
		vec3 albedo,
		float metallic, 
		vec3 normal,
		float roughness,
		vec3 viewDir,
		vec3 lightDir,
		vec3 reflectionDir,
		float shadow,
		float ssaoAmbientOcclusion,
        out vec3 directLighting) {

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = pbrDirectLight(viewDir, normal, lightDir, roughness, F0, metallic, albedo);
    
    vec3 ambient =  pbrAmbientLight(viewDir, normal, roughness, F0, metallic, albedo, reflectionDir, ao);
	
    float NdotL = max(dot(lightDir, normal), 0.0);   
    
	//if (NdotL == 0.0) {
        //shadow = clamp(shadow + 1.0, 0.0, 1.0);
    //}
	
	//shadow = clamp(shadow, 0.3f, 1.0f);
	
	//float ssaoFactor = max(max(ambient.r, ambient.g), ambient.b);
	//ssaoFactor = clamp (1 / ssaoFactor, 0, 1);
	
    vec3 color = ambient; //* ambientShadow; // ssaoAmbientOcclusion;
    
    
    float ambientShadow = clamp(shadow, 1.0 - shadowStrength, 1.0);
    color -= color*(1.0 - ambientShadow);
	
	// shadows affecting only direct light contribution
	//color += Lo * shadow;
    directLighting = shadow * Lo;
	color += directLighting;
	//color *= shadow;
	
	//ssaoAmbientOcclusion = pow(ssaoAmbientOcclusion, 2.2);
	
	color *= ssaoAmbientOcclusion;
	
	return color;
}

vec3 pbrDirectLight(vec3 V, vec3 N, vec3 L, float roughness, vec3 F0, float metallic, vec3 albedo) {
	
	vec3 H = normalize(V + L);
	
	// directional lights have no distance and thus also no attenuation
	//float distance = length(lightPosition - fragPos);
	//float attenuation = 1.0 / (distance * distance);
	
	//243 159 24
	
	//vec3 radiance = vec3(243/ 255.0f, 159 / 255.0f, 24 / 255.0f) * 1.0f;//dirLight.color; /** attenuation*/
	vec3 radiance = dirLight.color * dirLight.power;//dirLight.color; /** attenuation*/

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness);   
	float G   = GeometrySmith(N, V, L, roughness);    
	vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
	
	// specular reflection
	vec3 nominator    = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
	vec3 specular = nominator / denominator;
	
	/*float sDotN = max(dot(L, N), 0.0 );
    if( sDotN > 0.0 ) { 
		float shininess = pow( max( dot(N, H), 0.0 ), 4.0 ); 	
		//float shininess = pow( max( dot(r, viewDir), 0.0 ), 16.0 );
        specular = 5 * specular * shininess;	
	}*/
	
	 // kS is equal to Fresnel
	vec3 kS = F;
	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	vec3 kD = vec3(1.0) - kS;
	// multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - metallic;	                
		
	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);        

	// add to outgoing radiance Lo
	return (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again	
}

vec3 pbrAmbientLight(vec3 V, vec3 N, float roughness, vec3 F0, float metallic, vec3 albedo, vec3 R, float ao) {
	// ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 normalWorld = vec3(inverseViewMatrix_GPass * vec4(N, 0.0));
    
    //Important: We need world space normals! TODO: Maybe it is possible to generate 
    // irridianceMap in such a way, that we can use view space normals, too.
    vec3 irradiance = texture(irradianceMap, normalWorld).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
	
    // Important: R has to be in world space, too.
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	//brdf = vec2(1.0, 0.0);
	//brdf = vec2(1,1);
    vec3 ambientLightSpecular = prefilteredColor * (F * brdf.x + brdf.y);

    return ambientLightPower * (kD * diffuse + ambientLightSpecular) * ao;
}


// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   
// ----------------------------------------------------------------------------