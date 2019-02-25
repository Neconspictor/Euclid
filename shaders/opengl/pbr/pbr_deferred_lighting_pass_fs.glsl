#version 430

#include "shadow/shadows_array.glsl"

#define NUM_CASCADES 4

out vec4 FragColor;

const float PI = 3.14159265359;

struct GBuffer {
    sampler2D albedoMap;
	sampler2D aoMetalRoughnessMap;
	sampler2D normalEyeMap;
    sampler2D depthMap;
};


struct DirLight {
    vec3 directionEye;
    vec3 color;
};

struct CascadeData {
	//mat4 viewMatrix;
	mat4 inverseViewMatrix;
	mat4 lightViewProjectionMatrices[NUM_CASCADES];
    vec4 scaleFactors[NUM_CASCADES];
	vec4 cascadedSplits[NUM_CASCADES];
};


in VS_OUT {	
	vec2 tex_coords;
} fs_in;

uniform DirLight dirLight;

uniform GBuffer gBuffer;
//uniform sampler2D shadowMap;
uniform sampler2D ssaoMap;

uniform mat4 inverseViewMatrix_GPass; // the inverse view from the geometry pass!
uniform mat4 eyeToLight;
uniform mat4 viewGPass;


// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;


// Cascaded shadow mapping
layout(std140,binding=0) uniform CascadeBuffer {
	CascadeData cascadeData;
};

uniform sampler2DArray cascadedDepthMap;

uniform mat4 inverseProjMatrix_GPass;


vec3 pbrModel(float ao,
		vec3 albedo,
		float metallic, 
		vec3 normal,
		float roughness,
		vec3 viewDir,
		vec3 lightDir,
		vec3 reflectionDir,
		float shadow,
		float ssaoAmbientOcclusion);


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

uint getCascadeIdx(float viewSpaceZ) {
    uint cascadeIdx = 0;
    
    const float positiveZ = -viewSpaceZ;

    // Figure out which cascade to sample from
    for(uint i = 0; i < NUM_CASCADES - 1; ++i)
    {
        if(positiveZ > cascadeData.cascadedSplits[i].x)
        {	
            cascadeIdx = i + 1;
        }
    }
    
    return cascadeIdx;
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
	
	
	//vec3 positionEye = texture(gBuffer.positionEyeMap, fs_in.tex_coords).rgb;
    float depth = texture(gBuffer.depthMap, fs_in.tex_coords).r;
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
    //reflectionDir  = vec3(0,0,0);
	//vec3 reflectionDir = reflect(-viewEye, normalEye);
	//reflectionDir = vec3(inverseViewMatrix_GPass * vec4(reflectionDir, 0.0f)); // reflectionDir needs to be in world space
	
	//directional shadow calculation
	vec4 positionLight = eyeToLight * vec4(positionEye.rgb, 1.0);
	//float shadow = shadowCalculation(shadowMap, lightEye, normalEye, positionLight);
	//cascadedShadow(vec3 lightDirection, vec3 normal, float depthViewSpace,vec3 viewPosition)
	//float shadow = cascadedShadow(-dirLight.directionEye, normalEye, positionEye.z, positionEye);
	float shadow = cascadedShadow(-dirLight.directionEye, normalEye, positionEye.z, positionEye);
	//shadow = 1;
	
	
    vec3 result = pbrModel(ao, 
		albedo, 
		metallic, 
		normalEye, 
		roughness, 
		viewEye, 
		lightEye, 
		reflectionDir,
		shadow,
		ambientOcclusion);

	
	//alpha = clamp(alpha, 0, 1);
	
	FragColor = vec4(result, 1);
    
    
    uint cascadeIdx = getCascadeIdx(positionEye.z);
    cascadeIdx = 10;
    
    vec4 cascadeColor = FragColor;
    
    if (cascadeIdx == 0) {
    
       cascadeColor = vec4(1,0,0,1); 
    
    } else if (cascadeIdx == 1) {
        cascadeColor = vec4(0,1,0,1); 
    } else if (cascadeIdx == 2) {
        cascadeColor = vec4(0,0,1,1); 
    };
    
    FragColor = 0.5*cascadeColor * 0.5*FragColor;
    
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
		float ssaoAmbientOcclusion) {

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = pbrDirectLight(viewDir, normal, lightDir, roughness, F0, metallic, albedo);
	vec3 ambient =  pbrAmbientLight(viewDir, normal, roughness, F0, metallic, albedo, reflectionDir, ao);
	
	float ambientShadow = clamp(shadow, 1.0, 1.0);
	
	shadow = clamp(shadow, 0.2f, 1.0f);
	
	//float ssaoFactor = max(max(ambient.r, ambient.g), ambient.b);
	//ssaoFactor = clamp (1 / ssaoFactor, 0, 1);
	
    vec3 color = ambient; //* ambientShadow; // ssaoAmbientOcclusion;
	
	// shadows affecting only direct light contribution
	//color += Lo * shadow;
	color += Lo;
	color *= shadow;
	
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
	vec3 radiance = dirLight.color * 1.0f;//dirLight.color; /** attenuation*/

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

    return (kD * diffuse + ambientLightSpecular) * ao; //ambientLightSpecular * 0.4
	//return prefilteredColor;
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




float cascadedShadow(vec3 lightDirection, vec3 normal, float depthViewSpace,vec3 viewPosition)
{
	
	float sDotN = dot(lightDirection, normal);
	
	// assure that fragments with a normal facing away from the light source 
	// are always in shadow (reduces unwanted unshadowing).
	if (sDotN < 0) {
		return 0;
	}
	
    // Figure out which cascade to sample from
	uint cascadeIdx = getCascadeIdx(depthViewSpace);
    
	float angleBias = 0.006f;

	mat4 lightViewProjectionMatrix = cascadeData.lightViewProjectionMatrices[cascadeIdx];

	vec4 fragmentModelViewPosition = vec4(viewPosition,1.0f);

	vec4 fragmentModelPosition = cascadeData.inverseViewMatrix * fragmentModelViewPosition;

	vec4 fragmentShadowPosition = lightViewProjectionMatrix * fragmentModelPosition;

	vec3 projCoords = fragmentShadowPosition.xyz /= fragmentShadowPosition.w;

	//Remap the -1 to 1 NDC to the range of 0 to 1
	projCoords = projCoords * 0.5f + 0.5f;

	// Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

	projCoords.z = cascadeIdx;   

	float bias = max(angleBias * (1.0 - dot(normal, lightDirection)), 0.0008);
	//vec2 texelSize = vec2(1.0)/textureSize(cascadedDepthMap, 0);
	vec2 texelSize = 1.0 / textureSize(cascadedDepthMap, 0).xy;
	float minBias = max(texelSize.x,texelSize.y);
	bias =  9 * minBias / cascadeData.scaleFactors[cascadeIdx].x;
    //bias = minBias;

	float shadow = 0.0;
	//vec2 texelSize = 1.0 / textureSize(cascadedDepthMap, 0).xy;
	
	float xSamples = 0;
	float ySamples = 0;
	float sampleCount = (2*xSamples + 1) * (2*ySamples + 1);
	
	/*float depth = texture2DArray(cascadedDepthMap, vec3(projCoords.xy, projCoords.z)).r;
	float diff =  abs(currentDepth - depth);
	float penumbraSize = diff / (minBias);
	penumbraSize = clamp(penumbraSize, 0, 1);
	penumbraSize = (NUM_CASCADES - projCoords.z) / NUM_CASCADES;
	penumbraSize = NUM_CASCADES * projCoords.z;*/
	float penumbraSize = 1.0;
	vec2 size = textureSize(cascadedDepthMap, 0).xy;
	
		
    for(float x=-xSamples; x<=xSamples; x += 1){
        for(float y=-ySamples; y<=ySamples; y += 1){
            vec2 off = vec2(x,y)/size * penumbraSize;
			vec2 uv = projCoords.xy + off;
			float compare = currentDepth - bias;
			//float shadowSample =  shadowLerp(cascadedDepthMap, size, uv, projCoords.z, currentDepth, bias, penumbraSize);
            float shadowSample = shadowCompare(cascadedDepthMap, vec4(uv, projCoords.z, currentDepth - bias));
            shadow += shadowSample;
        }
    }
    
    return shadow / (sampleCount);

	//float pcfDepth =  shadow2DArray(cascadedDepthMap, vec4(projCoords.xyz, currentDepth + bias )).r; 
	//shadow += currentDepth  > pcfDepth ? 0.0  : 1.0;
	//return shadow; 
}