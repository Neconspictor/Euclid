#ifndef CSM_CASCADE_DEPTH_MAP_BINDING_POINT
#define CSM_CASCADE_DEPTH_MAP_BINDING_POINT 9
#endif

#ifndef VOXEL_TEXTURE_BINDING_POINT
#define VOXEL_TEXTURE_BINDING_POINT 10
#endif

#ifndef PBR_IRRADIANCE_BINDING_POINT
#define PBR_IRRADIANCE_BINDING_POINT 6 
#endif

#ifndef PBR_PREFILTERED_BINDING_POINT
#define PBR_PREFILTERED_BINDING_POINT 7
#endif

#ifndef PBR_BRDF_LUT_BINDING_POINT
#define PBR_BRDF_LUT_BINDING_POINT 8
#endif

#ifndef PBR_IRRADIANCE_OUT_MAP_BINDINGPOINT
#define PBR_IRRADIANCE_OUT_MAP_BINDINGPOINT 11
#endif

#ifndef PBR_AMBIENT_REFLECTION_OUT_MAP_BINDINGPOINT
#define PBR_AMBIENT_REFLECTION_OUT_MAP_BINDINGPOINT 12
#endif


#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#define BUFFERS_DEFINE_MATERIAL_BUFFER 1
#include "interface/buffers.h"
#include "shadow/cascaded_shadow.glsl"
#include "pbr/viewspaceNormalization.glsl"
#include "interface/light_interface.h"
#include "interface/cluster_interface.h"
#include "util/spherical_harmonics.glsl"

//const float PI = 3.14159265359;
const float FLT_MAX = 3.402823466e+38;

uniform DirLight dirLight;

uniform float ambientLightPower;
uniform float shadowStrength;

// IBL
layout(binding = PBR_IRRADIANCE_BINDING_POINT)  uniform sampler1DArray irradianceMaps;
layout(binding = PBR_PREFILTERED_BINDING_POINT) uniform samplerCubeArray reflectionMaps;
layout(binding = PBR_BRDF_LUT_BINDING_POINT)    uniform sampler2D brdfLUT;


layout(binding = VOXEL_TEXTURE_BINDING_POINT) uniform sampler3D voxelTexture;
layout(binding = PBR_IRRADIANCE_OUT_MAP_BINDINGPOINT)    uniform sampler2D irradianceOutMap;
layout(binding = PBR_AMBIENT_REFLECTION_OUT_MAP_BINDINGPOINT)    uniform sampler2D ambientReflectionOutMap;

#include "GI/cone_trace.glsl"




struct ArrayIndexWeight {
    float indices[2];
    float weights[2];
};





vec3 pbrDirectLight(in vec3 V, in vec3 N, in vec3 L, in float roughness, in vec3 F0, in float metallic, in vec3 albedo);
vec3 pbrAmbientLight(in  vec3 normalWorld, in float roughness, in float metallic, in vec3 albedo,  in float ao,  in vec3 positionWorld, in vec3 viewWorld, in int irradianceArrayIndex, in int reflectionArrayIndex);
vec3 pbrAmbientLight2(in vec3 normalWorld, in float roughness, in float metallic, in vec3 albedo, in float ao, in vec3 viewWorld, in vec3 irradiance, in vec3 ambientReflection);
vec4 pbrIrradiance(in vec3 normalWorld, in vec3 positionWorld, in PerObjectMaterialData objectMaterialData);
vec4 pbrAmbientReflection(in vec3 normalWorld, in float roughness, in float metallic, in vec3 albedo, in float ao, in vec3 positionWorld, in vec3 viewWorld, in PerObjectMaterialData objectMaterialData);


float DistributionGGX(in vec3 N, in vec3 H, in float roughness);
float GeometrySchlickGGX(in float NdotV, in float roughness);
float GeometrySmith(in vec3 N, in vec3 V, in vec3 L, in float roughness);
vec3 fresnelSchlick(in float cosTheta, in vec3 F0);
vec3 fresnelSchlickRoughness(in float cosTheta, in vec3 F0, in float roughness);
ArrayIndexWeight calcArrayIndices(in vec3 positionEye, in vec3 normalWorld, in vec2 texCoord);

void calcDirectLighting(in float ao, 
             in vec3 albedo, 
             in float metallic, 
             in vec3 normalWorld,
             in vec3 positionEye,
			 in float roughness,
             in vec3 viewWorld,             
             out vec3 colorOut,
             out vec3 luminanceOut) 
{
        // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = pbrDirectLight(viewWorld, normalWorld, dirLight.directionWorld.xyz, roughness, F0, metallic, albedo);
   // vec3 ambient = calcAmbientLighting(normalEye, positionEye, ao, albedo, metallic, roughness);
    
    float fragmentLitProportion = cascadedShadow(dirLight.directionWorld.xyz, normalWorld, positionEye.z, positionEye);
    vec3 directLighting = fragmentLitProportion * Lo;
    
    colorOut = directLighting;// + ambient;
    luminanceOut = 0.01 * directLighting;
}


vec3 pbrDirectLight(
	in vec3 V, 
	in vec3 N, 
	in vec3 L,
	in float roughness, 
	in vec3 F0, 
	in float metallic, 
	in vec3 albedo) {
	
	vec3 H = normalize(V + L);
	
	// directional lights have no distance and thus also no attenuation
	//float distance = length(lightPosition - fragPos);
	//float attenuation = 1.0 / (distance * distance);
	
	
	//vec3 radiance = vec3(243/ 255.0f, 159 / 255.0f, 24 / 255.0f) * 1.0f;//dirLight.color; /** attenuation*/
	vec3 radiance = dirLight.color.xyz * dirLight.power;//dirLight.color; /** attenuation*/

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

vec3 pbrAmbientLight(in vec3 normalWorld, in float roughness, in float metallic, 
in vec3 albedo, in float ao, in vec3 positionWorld, in vec3 viewWorld, in int irradianceArrayIndex, in int reflectionArrayIndex) {
	// ambient lighting (we now use IBL as the ambient term)
    
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

	vec3 reflectionDirWorld = normalize(reflect(-viewWorld, normalWorld));

	
	vec3 F = fresnelSchlickRoughness(max(dot(normalWorld, viewWorld), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
   // ArrayIndexWeight indexWeight = calcArrayIndices(positionEye, normalWorld);
    
    
    
    //Important: We need world space normals! TODO: Maybe it is possible to generate 
    // irradianceMap in such a way, that we can use view space normals, too.
    //vec3 irradiance = texture(irradianceMap, normalWorld).rgb;
    
    #if USE_CONE_TRACING
        vec4 coneTracedIrradiance = ConeTraceRadiance(positionWorld, normalWorld);
        vec3 irradiance = coneTracedIrradiance.a * coneTracedIrradiance.rgb;
    #else 
        vec3 irradiance =  computeIrradiance(irradianceMaps, irradianceArrayIndex, normalWorld);
    #endif
    
    //irradiance1 = vec3(1 - indexWeight.indices[0],0, indexWeight.indices[0]);
    //vec3 irradiance2 = texture(irradianceMaps, vec4(normalWorld, indexWeight.indices[1])).rgb;
    //irradiance2 = vec3(1 - indexWeight.indices[1],0, indexWeight.indices[1]);
    
    //vec3 irradiance = indexWeight.firstWeight * irradiance1 + (1.0-indexWeight.firstWeight) * irradiance2;
    //vec3 irradiance = indexWeight.weights[0] * irradiance1 + (indexWeight.weights[1]) * irradiance2;
    
	//vec3 testNormal = normalWorld;
	//testNormal.x *= 5.0;
	//testNormal.y *= 5.0;
	
	float angle = max(dot(normalWorld, viewWorld), 0.0);
	angle = 1;
	
    vec3 diffuse      =  angle * irradiance * albedo; //TODO: multiply by angle is valid pbr?
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 7.0;
    
    
    
	
    // Important: R has to be in world space, too.
    //vec3 prefilteredColor = textureLod(reflectionMaps, vec4(reflectionDirWorld, 0), roughness * MAX_REFLECTION_LOD).rgb;
    vec3 prefilteredColor = vec3(0.0);
    
    #if USE_CONE_TRACING
        vec4 coneTracedReflection = ConeTraceReflection(positionWorld, normalWorld, viewWorld, 1.0 - roughness);
        prefilteredColor = coneTracedReflection.a * coneTracedReflection.rgb;
    #else 
	
        prefilteredColor = textureLod(reflectionMaps, vec4(reflectionDirWorld, reflectionArrayIndex), 
										roughness * MAX_REFLECTION_LOD).rgb;							
    #endif
    
    //ConeTraceReflection
    
    /*vec3 prefilteredColor1 = textureLod(reflectionMaps, vec4(reflectionDirWorld, indexWeight.indices[0]), roughness * MAX_REFLECTION_LOD).rgb;
    
    vec3 prefilteredColor2 = textureLod(reflectionMaps, vec4(reflectionDirWorld, indexWeight.indices[1]), roughness * MAX_REFLECTION_LOD).rgb;
    //vec3 prefilteredColor = indexWeight.firstWeight * prefilteredColor1 + (1.0-indexWeight.firstWeight) * prefilteredColor2;
    vec3 prefilteredColor = indexWeight.weights[0] * prefilteredColor1 + (indexWeight.weights[1]) * prefilteredColor2;
    */
    
    //prefilteredColor = vec3(0.31985, 0.39602, 0.47121);
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(normalWorld, viewWorld), 0.0), roughness)).rg;
	//brdf = vec2(1.0, 0.0);
	//brdf = vec2(1,1);
    vec3 ambientLightSpecular = prefilteredColor * (F * brdf.x + brdf.y);

    return ambientLightPower * (kD * diffuse + ambientLightSpecular) * ao;
}

vec3 pbrAmbientLight2(in vec3 normalWorld, in float roughness, in float metallic, 
in vec3 albedo, in float ao, in vec3 viewWorld, in vec3 irradiance, in vec3 ambientReflection) {
	
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	
    vec3 F = fresnelSchlickRoughness(max(dot(normalWorld, viewWorld), 0.0), F0, roughness);
    
	
	float angle = max(dot(normalWorld, viewWorld), 0.0);
	angle = 1;
	
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 diffuse      =  angle * irradiance * albedo; // TODO: multiply by angle is valid pbr?
    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(normalWorld, viewWorld), 0.0), roughness)).rg;
    vec3 ambientLightSpecular = ambientReflection * (F * brdf.x + brdf.y);

    //return ambientLightPower * (kD * diffuse + ambientLightSpecular) * ao;
	return ambientLightPower * (kD * diffuse + ambientLightSpecular) * ao;
}

vec4 pbrIrradiance(in vec3 normalWorld, in vec3 positionWorld, in PerObjectMaterialData objectMaterialData) {
    
	#if USE_CONE_TRACING
		vec4 irradiance = ConeTraceRadiance(positionWorld, normalWorld);
		irradiance.xyz *= intBitsToFloat(objectMaterialData.coneTracing.y);
        
    #else
	
		vec4 irradiance = vec4(0,0,0,1);
	
		for (int i = 0; i < 9; ++i) {
			const float factor = getCosineLobeFactorSH(i);
			const float Ylm = harmonicsSH(i, normalWorld);
			const vec3 Llm = objectMaterialData.diffuseSHCoefficients[i].xyz;
			irradiance.xyz += factor * Llm * Ylm;
		}

		irradiance.xyz *= intBitsToFloat(objectMaterialData.probes.y);
		
    #endif

    return irradiance;
}


vec4 pbrAmbientReflection(in vec3 normalWorld, in float roughness,
in float metallic, in vec3 albedo, in float ao, in vec3 positionWorld, in vec3 viewWorld, in PerObjectMaterialData objectMaterialData) {
	// ambient lighting (we now use IBL as the ambient term)
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	
    vec3 F = fresnelSchlickRoughness(max(dot(normalWorld, viewWorld), 0.0), F0, roughness);
	
	vec3 reflectionDirWorld = normalize(reflect(-viewWorld, normalWorld));
      
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 10.0;
    vec4 prefilteredColor = vec4(0,0,0,0);
	
	
    #if USE_CONE_TRACING
			prefilteredColor += roughness * ConeTraceReflection(positionWorld, normalWorld, viewWorld, roughness);
			
			prefilteredColor.xyz *= intBitsToFloat(objectMaterialData.coneTracing.y);
    #else 
		for (int i = 0; i < 4; ++i) {
			prefilteredColor += objectMaterialData.specularReflectionWeights[i] * vec4(textureLod(reflectionMaps, vec4(reflectionDirWorld, objectMaterialData.specularReflectionIds[i]), 
										roughness * MAX_REFLECTION_LOD).rgb, 1.0);
		}

		prefilteredColor.xyz *= intBitsToFloat(objectMaterialData.probes.y);
		
    #endif

    return prefilteredColor;
}


// ----------------------------------------------------------------------------
float DistributionGGX(in vec3 N, in vec3 H, in float roughness)
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
float GeometrySchlickGGX(in float NdotV, in float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(in vec3 N, in vec3 V, in vec3 L, in float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(in float cosTheta, in vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(in float cosTheta, in vec3 F0, in float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   
// ----------------------------------------------------------------------------