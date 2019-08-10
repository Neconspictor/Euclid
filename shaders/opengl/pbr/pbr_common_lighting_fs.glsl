#extension GL_ARB_bindless_texture : require

#ifndef CSM_CASCADE_BUFFER_BINDING_POINT
#define CSM_CASCADE_BUFFER_BINDING_POINT 0
#endif

#ifndef CSM_CASCADE_DEPTH_MAP_BINDING_POINT
#define CSM_CASCADE_DEPTH_MAP_BINDING_POINT 8
#endif

// Note: uniform buffers are different from shader storage buffers!
#ifndef PBR_PROBES_BUFFER_BINDING_POINT
#define PBR_PROBES_BUFFER_BINDING_POINT 1 
#endif

#include "shadow/cascaded_shadow.glsl"
#include "pbr/viewspaceNormalization.glsl"
#include "interface/light_interface.h"


const float PI = 3.14159265359;
const float FLT_MAX = 3.402823466e+38;

/*struct Probe {
    samplerCube irradianceMap;
    samplerCube prefilterMap;
};*/

uniform DirLight dirLight;

uniform float ambientLightPower;
uniform float shadowStrength;

// The inverse view matrix. Note, for deferred renderings the inverse view of the geometry pass is meant!
uniform mat4 inverseViewMatrix;

// IBL
layout(binding = 5) uniform samplerCubeArray irradianceMaps;
layout(binding = 6) uniform samplerCubeArray prefilteredMaps;
layout(binding = 7) uniform sampler2D brdfLUT;
uniform float arrayIndex; //Note: an unsigned integer value represented as a float value


struct Probe {
    // first component: array index, second component: influence radius
    vec4 indexInfluenceRadius;
    vec4 positionWorld; // last component isn't used
};

layout(std430, binding = PBR_PROBES_BUFFER_BINDING_POINT) buffer ProbesBlock {
    Probe probesData[]; // Each probe will be aligned to a multiple of vec4 
};

struct ArrayIndexWeight {
    float firstIndex;
    float firstWeight;
    float secondIndex;
    float secondWeight;
};



vec3 pbrDirectLight(vec3 V, vec3 N, float roughness, vec3 F0, float metallic, vec3 albedo);
vec3 pbrAmbientLight(vec3 V, vec3 N, vec3 normalWorld, float roughness, vec3 F0, float metallic, vec3 albedo, vec3 reflectionDirWorld, float ao, vec3 positionWorld);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
ArrayIndexWeight calcArrayIndices(in vec3 positionWorld);


void calcLighting(in float ao, 
             in vec3 albedo, 
             in float metallicOriginal, 
             in vec3 normalEye, 
             in float roughnessOrignal,
             in vec3 positionEye,             
             //in vec2 texCoord,
             out vec3 colorOut,
             out vec3 luminanceOut) 
{
    
    float roughness = roughnessOrignal;
    float metallic = metallicOriginal;
    
    // view direction
	vec3 viewEye = normalize(-positionEye);
    
    
    
	// reflection direction
    vec3 viewWorld = vec3(inverseViewMatrix * vec4(viewEye, 0.0f));
    vec3 normalWorld = normalize(vec3(1.0, 1.0, 1.0) * vec3(inverseViewMatrix * vec4(normalEye, 0.0f)));
    vec3 normalEx = normalEye;//normalize(vec3(inverse(inverseViewMatrix) * vec4(normalWorld, 0.0f)));
    vec3 reflectionDirWorld = normalize(reflect(viewWorld, normalWorld));

    
    //metallic = 1.0;
    //roughness = 0.0;

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = pbrDirectLight(viewEye, normalEx, roughness, F0, metallic, albedo);
    
    
    vec3 positionWorld = vec3(inverseViewMatrix * vec4(positionEye, 1.0f));
    vec3 ambient =  pbrAmbientLight(viewEye, normalEx, normalWorld, roughness, F0, metallic, albedo, reflectionDirWorld, ao, positionWorld);
    
    float fragmentLitProportion = cascadedShadow(-dirLight.directionEye, normalEx, positionEye.z, positionEye);
	
    vec3 color = ambient;// + albedo * 0.01 * ambientLightPower; //* ambientShadow; // ssaoAmbientOcclusion;
    float ambientShadow = clamp(fragmentLitProportion, 1.0 - shadowStrength, 1.0);
    color -= color*(1.0 - ambientShadow);
	
	// shadows affecting only direct light contribution
	//color += Lo * shadow;
    vec3 directLighting = fragmentLitProportion * Lo;
    
	color += directLighting;
    
    colorOut = color;
    luminanceOut = 0.1 * directLighting;
    
    /*return;
    
    //Debug
    uint cascadeIdx = getCascadeIdx(positionEye.z, cascadeData);
    cascadeIdx = 10;
    
    vec3 cascadeColor = colorOut;
    
    if (cascadeIdx == 0) {
       cascadeColor = vec3(1,0,0); 
    } else if (cascadeIdx == 1) {
        cascadeColor = vec3(0,1,0); 
    } else if (cascadeIdx == 2) {
        cascadeColor = vec3(0,0,1); 
    };
    
    colorOut = 0.5*cascadeColor + 0.5*colorOut;*/
}


vec3 pbrDirectLight(vec3 V, vec3 N, float roughness, vec3 F0, float metallic, vec3 albedo) {
	
    vec3 L = normalize(-dirLight.directionEye);
	vec3 H = normalize(V + L);
	
	// directional lights have no distance and thus also no attenuation
	//float distance = length(lightPosition - fragPos);
	//float attenuation = 1.0 / (distance * distance);
	
	
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
	float NdotL = clamp(0.0, 1.0, max(dot(N, L), 0.0));        

	// add to outgoing radiance Lo
	return (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again	
}

vec3 pbrAmbientLight(vec3 V, vec3 N, vec3 normalWorld, float roughness, vec3 F0, float metallic, vec3 albedo, vec3 reflectionDirWorld, float ao, vec3 positionWorld) {
	// ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    ArrayIndexWeight indexWeight = calcArrayIndices(positionWorld);
    
    
    
    //Important: We need world space normals! TODO: Maybe it is possible to generate 
    // irradianceMap in such a way, that we can use view space normals, too.
    //vec3 irradiance = texture(irradianceMap, normalWorld).rgb;
    vec3 irradiance1 = texture(irradianceMaps, vec4(normalWorld, indexWeight.firstIndex)).rgb;
    vec3 irradiance2 = texture(irradianceMaps, vec4(normalWorld, indexWeight.secondIndex)).rgb;
    
    //vec3 irradiance = indexWeight.firstWeight * irradiance1 + (1.0-indexWeight.firstWeight) * irradiance2;
    vec3 irradiance = indexWeight.firstWeight * irradiance1 + (indexWeight.secondWeight) * irradiance2;
    
    vec3 diffuse      =  irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 7.0;
    
    
    
	
    // Important: R has to be in world space, too.
    //vec3 prefilteredColor = textureLod(prefilterMap, reflectionDirWorld, roughness * MAX_REFLECTION_LOD).rgb;
    vec3 prefilteredColor1 = textureLod(prefilteredMaps, vec4(reflectionDirWorld, indexWeight.firstIndex), roughness * MAX_REFLECTION_LOD).rgb;
    vec3 prefilteredColor2 = textureLod(prefilteredMaps, vec4(reflectionDirWorld, indexWeight.secondIndex), roughness * MAX_REFLECTION_LOD).rgb;
    //vec3 prefilteredColor = indexWeight.firstWeight * prefilteredColor1 + (1.0-indexWeight.firstWeight) * prefilteredColor2;
    vec3 prefilteredColor = indexWeight.firstWeight * prefilteredColor1 + (indexWeight.secondWeight) * prefilteredColor2;
    
    
    //prefilteredColor = vec3(0.31985, 0.39602, 0.47121);
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	//brdf = vec2(1.0, 0.0);
	//brdf = vec2(1,1);
    vec3 ambientLightSpecular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 withoutRoughness = ambientLightPower * (kD * diffuse + ambientLightSpecular) * ao;
    vec3 fullRoughness = ambientLightPower * (kD * diffuse) * ao;
    
    //return mix(withoutRoughness, fullRoughness, roughness);
    return withoutRoughness;
}

ArrayIndexWeight calcArrayIndices(in vec3 positionWorld) {

  float minDistance = FLT_MAX;
  float minDistance2 = FLT_MAX;
  float arrayIndex = FLT_MAX;
  float arrayIndex2 = FLT_MAX;


  Probe probeData1 = probesData[0];
  arrayIndex = probeData1.indexInfluenceRadius.x;
  minDistance = length(probeData1.positionWorld.xyz - positionWorld);
  
  Probe probeData2 = probesData[1];
  arrayIndex2 = probeData2.indexInfluenceRadius.x;
  minDistance2 = length(probeData2.positionWorld.xyz - positionWorld);
  
  float distanceDiff = length(minDistance - minDistance2);
  
  ArrayIndexWeight result;
  result.firstIndex = arrayIndex;
  result.secondIndex = arrayIndex2;
  
  
  vec3 normal = normalize(probeData2.positionWorld.xyz - probeData1.positionWorld.xyz);
  vec3 origin = probeData1.positionWorld.xyz + 0.5 * distanceDiff * normal;
  float signDist = dot(normal, origin);
  float signDistMin = dot(normal, positionWorld);
  float normalizedDistToPlane = abs(signDist - signDistMin) / distanceDiff;
  
  
  
  const float innerRadius = 2.0;
  const float outerRadiusDiff = 70.0;
  
  if (minDistance < innerRadius) {
  result.firstWeight = 1.0;
  } else {
  
    result.firstWeight = clamp(pow(max(1.0 - (minDistance - innerRadius) / outerRadiusDiff, 0.0), 2.0), 0, 1);
  }
  
  result.firstWeight = 1.0; 
  
   result.secondWeight = 1.0 - result.firstWeight;
  
  //if (minDistance2 < innerRadius) {
  //result.secondWeight = 1.0;
  //} else {
 // 
  //  result.secondWeight = pow(max(1.0 - (minDistance2 - innerRadius) / outerRadiusDiff, 0.0), 2.0);
  //}
  
  
  //normalize weights
  //const float weightSum = result.firstWeight + result.secondWeight;
  
  /*if (weightSum > 1) {
    result.firstWeight = clamp(0, 1, result.firstWeight / weightSum);
    result.secondWeight = clamp(0, 1, result.secondWeight / weightSum);
  }*/
  
  
  
  return result;
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