#extension GL_ARB_bindless_texture : require

#ifndef CSM_CASCADE_BUFFER_BINDING_POINT
#define CSM_CASCADE_BUFFER_BINDING_POINT 0
#endif

#ifndef CSM_CASCADE_DEPTH_MAP_BINDING_POINT
#define CSM_CASCADE_DEPTH_MAP_BINDING_POINT 8
#endif


#ifndef PBR_PROBES_BUFFER_BINDING_POINT
#define PBR_PROBES_BUFFER_BINDING_POINT 1 
#endif

#ifndef PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES
#define PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES 2 
#endif

#ifndef PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS
#define PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS 3
#endif

#ifndef PBR_CLUSTERS_AABB
#define PBR_CLUSTERS_AABB 4
#endif

// Note: uniform buffers are different from shader storage buffers!
#ifndef PBR_CONSTANTS
#define PBR_CONSTANTS 0
#endif

#ifndef VOXEL_C_UNIFORM_BUFFER_BINDING_POINT
#define VOXEL_C_UNIFORM_BUFFER_BINDING_POINT 1
#endif

#ifndef VOXEL_TEXTURE_BINDING_POINT
#define VOXEL_TEXTURE_BINDING_POINT 9
#endif

//Feature define macros:
// USE_CONE_TRACING


#include "shadow/cascaded_shadow.glsl"
#include "pbr/viewspaceNormalization.glsl"
#include "interface/light_interface.h"
#include "interface/cluster_interface.h"


const float PI = 3.14159265359;
const float FLT_MAX = 3.402823466e+38;

/*struct Probe {
    samplerCube irradianceMap;
    samplerCube prefilterMap;
};*/

struct PbrConstants {
    uvec4 windowDimension;
    uvec4 clusterDimension;
    vec4 nearFarDistance;
};

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


layout(std430, binding = PBR_PROBES_BUFFER_BINDING_POINT) buffer ProbesBlock {
    EnvironmentLight environmentLights[];
};

layout(std430, binding = PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES) buffer EnvLightGlobalLightIndicesBlock {
    uint globalLightIndexList[];
};

layout(std430, binding = PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS) buffer EnvLightLightGridsBlock {
    LightGrid lightGrids[];
};

layout(std430, binding = PBR_CLUSTERS_AABB) buffer ClustersAABBBlock {
    AABB clusters[];
};

layout(std140, binding = PBR_CONSTANTS) uniform ConstantsBlock {
    PbrConstants constants;
};

/**
 * Cone tracing
 */
layout(std140, binding = VOXEL_C_UNIFORM_BUFFER_BINDING_POINT) uniform Cbuffer {
    float       g_xFrame_VoxelRadianceDataSize;				// voxel half-extent in world space units
	float       g_xFrame_VoxelRadianceDataSize_rcp;			// 1.0 / voxel-half extent
    uint		g_xFrame_VoxelRadianceDataRes;				// voxel grid resolution
	float		g_xFrame_VoxelRadianceDataRes_rcp;			// 1.0 / voxel grid resolution
    
    uint		g_xFrame_VoxelRadianceDataMIPs;				// voxel grid mipmap count
	uint		g_xFrame_VoxelRadianceNumCones;				// number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceNumCones_rcp;			// 1.0 / number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceRayStepSize;			// raymarch step size in voxel space units
    
    vec4		g_xFrame_VoxelRadianceDataCenter;			// center of the voxel grid in world space units
	uint		g_xFrame_VoxelRadianceReflectionsEnabled;	// are voxel gi reflections enabled or not   
};

layout(binding = VOXEL_TEXTURE_BINDING_POINT) uniform sampler3D voxelTexture;

#include "GI/cone_trace.glsl"




struct ArrayIndexWeight {
    float indices[2];
    float weights[2];
};





vec3 pbrDirectLight(in vec3 V, in vec3 N, in float roughness, in vec3 F0, in float metallic, in vec3 albedo);
vec3 pbrAmbientLight(in  vec3 normalWorld, in float roughness, in vec3 F0, in float metallic, in vec3 albedo, in  vec3 reflectionDirWorld,  in float ao,  in vec3 positionWorld, in vec3 viewWorld);
float DistributionGGX(in vec3 N, in vec3 H, in float roughness);
float GeometrySchlickGGX(in float NdotV, in float roughness);
float GeometrySmith(in vec3 N, in vec3 V, in vec3 L, in float roughness);
vec3 fresnelSchlick(in float cosTheta, in vec3 F0);
vec3 fresnelSchlickRoughness(in float cosTheta, in vec3 F0, in float roughness);
ArrayIndexWeight calcArrayIndices(in vec3 positionEye, in vec3 normalWorld);

vec3 calcAmbientLighting(in vec3 normalEye, in vec3 positionEye, in float ao, in vec3 albedo, in float metallic, in float roughness);


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
    
    

    // reflectance equation
    vec3 Lo = pbrDirectLight(normalize(-positionEye), normalEye, roughness, F0, metallic, albedo);
    
    vec3 ambient = calcAmbientLighting(normalEye, positionEye, ao, albedo, metallic, roughness);
    
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




vec3 calcAmbientLighting(in vec3 normalEye, in vec3 positionEye, in float ao, in vec3 albedo, in float metallic, in float roughness) 
{    
	// reflection direction
    vec3 viewEye = normalize(-positionEye);
    vec3 viewWorld = normalize(vec3(inverseViewMatrix * vec4(viewEye, 0.0f)));
    vec3 normalWorld = normalize(vec3(1.0, 1.0, 1.0) * vec3(inverseViewMatrix * vec4(normalEye, 0.0f)));
    vec3 reflectionDirWorld = normalize(reflect(viewWorld, normalWorld));
    vec3 positionWorld = vec3(inverseViewMatrix * vec4(positionEye, 1.0f));

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    
    return pbrAmbientLight(normalWorld, roughness, F0, metallic, albedo, reflectionDirWorld, ao, positionWorld, viewWorld);    

}


vec3 pbrDirectLight(in vec3 V, in vec3 N, in float roughness, in vec3 F0, in float metallic, in vec3 albedo) {
	
    vec3 L = normalize(dirLight.directionEye);
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

vec3 pbrAmbientLight(in vec3 normalWorld, in float roughness, in vec3 F0, 
in float metallic, in vec3 albedo, in vec3 reflectionDirWorld, in float ao, in vec3 positionWorld, in vec3 viewWorld) {
	// ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(normalWorld, viewWorld), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
   // ArrayIndexWeight indexWeight = calcArrayIndices(positionEye, normalWorld);
    
    
    
    //Important: We need world space normals! TODO: Maybe it is possible to generate 
    // irradianceMap in such a way, that we can use view space normals, too.
    //vec3 irradiance = texture(irradianceMap, normalWorld).rgb;
    
    #ifdef USE_CONE_TRACING
        vec4 coneTracedIrradiance = ConeTraceRadiance(positionWorld, normalWorld);
        vec3 irradiance = coneTracedIrradiance.a * coneTracedIrradiance.rgb;
    #else 
        vec3 irradiance = texture(irradianceMaps, vec4(normalWorld, 0)).rgb;
    #endif
    
    //irradiance1 = vec3(1 - indexWeight.indices[0],0, indexWeight.indices[0]);
    //vec3 irradiance2 = texture(irradianceMaps, vec4(normalWorld, indexWeight.indices[1])).rgb;
    //irradiance2 = vec3(1 - indexWeight.indices[1],0, indexWeight.indices[1]);
    
    //vec3 irradiance = indexWeight.firstWeight * irradiance1 + (1.0-indexWeight.firstWeight) * irradiance2;
    //vec3 irradiance = indexWeight.weights[0] * irradiance1 + (indexWeight.weights[1]) * irradiance2;
    
    vec3 diffuse      =  irradiance * albedo + 0.01 * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 7.0;
    
    
    
	
    // Important: R has to be in world space, too.
    //vec3 prefilteredColor = textureLod(prefilteredMaps, vec4(reflectionDirWorld, 0), roughness * MAX_REFLECTION_LOD).rgb;
    vec3 prefilteredColor = vec3(0.0);
    
    #ifdef USE_CONE_TRACING
        vec4 coneTracedReflection = ConeTraceReflection(positionWorld, normalWorld, viewWorld, roughness);
        prefilteredColor = coneTracedReflection.a * coneTracedReflection.rgb;
    #else 
        prefilteredColor = textureLod(prefilteredMaps, vec4(reflectionDirWorld, 0), roughness * MAX_REFLECTION_LOD).rgb;
    #endif
    
    //ConeTraceReflection
    
    /*vec3 prefilteredColor1 = textureLod(prefilteredMaps, vec4(reflectionDirWorld, indexWeight.indices[0]), roughness * MAX_REFLECTION_LOD).rgb;
    
    vec3 prefilteredColor2 = textureLod(prefilteredMaps, vec4(reflectionDirWorld, indexWeight.indices[1]), roughness * MAX_REFLECTION_LOD).rgb;
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


bool testAABB(in vec3 point, in vec3 minVec, in vec3 maxVec) {
    
    return (point.x <= maxVec.x && minVec.x <= point.x) &&
    (point.y <= maxVec.y && minVec.y <= point.y) &&
    (point.z <= maxVec.z && minVec.z <= point.z);
}

float distanceAABB(in vec3 point, in vec3 minVec, in vec3 maxVec) 
{
  const float dx = max(max(minVec.x - point.x, 0), point.x - maxVec.x);
  const float dy = max(max(minVec.y - point.y, 0), point.y - maxVec.y);
  const float dz = max(max(minVec.z - point.z, 0), point.z - maxVec.z);
  return sqrt(dx*dx + dy*dy + dz*dz);
}


struct EnvLightData {
	float influence;
	float volume;
    uint lightID;
    uint arrayIndex;
  };
  
  
#define CSWAP_INFLUENCE(arr, indexA, indexB, tmp) \
if (arr[indexA].influence < arr[indexB].influence) { \
	tmp = arr[indexB]; \
	arr[indexB] = arr[indexA];\
	arr[indexA] = tmp;\
} 

#define CSWAP_VOLUME(arr, indexA, indexB, tmp) \
if (arr[indexA].volume > arr[indexB].volume && arr[indexB].influence > 0.0001) { \
	tmp = arr[indexB]; \
	arr[indexB] = arr[indexA];\
	arr[indexA] = tmp;\
}   

ArrayIndexWeight calcArrayIndices(in vec3 positionEye, in vec3 normalWorld) {

  vec3 positionWorld = vec3(inverseViewMatrix * vec4(positionEye, 1.0f));
  float logZ = log(-positionEye.z); 
  //z = 0.1; //just validation now

  //Getting the linear cluster index value
  
  const float nearDistance = float(constants.nearFarDistance.x);
  const float farDistance = float(constants.nearFarDistance.y);
  const float xSlices = float(constants.clusterDimension.x);
  const float ySlices = float(constants.clusterDimension.y);
  const float zSlices = float(constants.clusterDimension.z);

  float clusterZVal  = ((logZ * zSlices / log(farDistance / nearDistance)) - 
                        zSlices * log(nearDistance) / log(farDistance/ nearDistance));
  
  uint clusterZ = uint(clusterZVal);
  vec2 clusterPixelSize = vec2(constants.windowDimension.x , constants.windowDimension.y) / vec2(xSlices , ySlices);
  vec2 pixelLoc = vec2(constants.windowDimension.x  * fs_in.tex_coords.x, constants.windowDimension.y * (fs_in.tex_coords.y));
  uvec3 clusters    = uvec3( uvec2(pixelLoc / clusterPixelSize), clusterZ);
  uint clusterID = clusters.x +
                   constants.clusterDimension.x * clusters.y +
                   constants.clusterDimension.x * constants.clusterDimension.y * clusters.z;


  /*float minDistance = FLT_MAX;
  float minDistance2 = FLT_MAX;
  float arrayIndex = FLT_MAX;
  float arrayIndex2 = FLT_MAX;


  EnvironmentLight envLight1 = environmentLights[0];
  arrayIndex = envLight1.arrayIndex;
  minDistance = length(envLight1.position.xyz - positionWorld);
  
  EnvironmentLight envLight2 = environmentLights[1];
  arrayIndex2 = envLight2.arrayIndex;
  minDistance2 = length(envLight2.position.xyz - positionWorld);
  
  float distanceDiff = length(minDistance - minDistance2);
  
  ArrayIndexWeight result;
  result.firstIndex = arrayIndex;
  result.secondIndex = arrayIndex2;
  
  
  vec3 normal = normalize(envLight2.position.xyz - envLight1.position.xyz);
  vec3 origin = envLight1.position.xyz + 0.5 * distanceDiff * normal;
  float signDist = dot(normal, origin);
  float signDistMin = dot(normal, positionWorld);
  float normalizedDistToPlane = abs(signDist - signDistMin) / distanceDiff;
  
  
  const float innerRadiusPercentage = 0.5;
  const float innerRadius = envLight1.sphereRange * innerRadiusPercentage;
  const float outerRadiusDiff = envLight1.sphereRange - innerRadius;
  
  //if (minDistance < innerRadius) {
  float dist = distanceAABB(positionWorld, envLight1.minWorld.rgb, envLight1.maxWorld.rgb);
  
  result.firstWeight = clamp(1.0 / (1.0 + 100.0 * dist + 3.0 * dist * dist + + 4.0 * dist * dist * dist), 0.0, 1.0);
  if (result.firstWeight < 0.01)
    result.firstWeight = 0.0;
  
  */
  
  /*if (testAABB(positionWorld, envLight1.minWorld.rgb, envLight1.maxWorld.rgb)) {
    result.firstWeight = 1.0;
  } else {
    result.firstWeight = 0.0;
    //result.firstWeight = clamp(pow(max(1.0 - (minDistance - innerRadius) / outerRadiusDiff, 0.0), 2.0), 0, 1);
  }*/
  
  //vec3 vec = normalize(envLight1.position.xyz - positionWorld);
  
  //float irradiance1MaxDistance = abs(textureLod(prefilteredMaps, vec4(normalize(-vec), arrayIndex), 0).r);
  
   //result.secondWeight = 1.0 - result.firstWeight;
  
  
  //clusterID
  LightGrid lightGrid = lightGrids[clusterID];

  
  EnvLightData lightDataSource[4];
  
  for (uint i = 0; i < 4; ++i) {
    lightDataSource[i].influence = 0.0;
    lightDataSource[i].volume = 0.0;
    lightDataSource[i].lightID = 0;
    lightDataSource[i].arrayIndex = 0;
  }
  
  for (int i = 0; i < lightGrid.count; ++i) {
  
    const uint lightID = globalLightIndexList[lightGrid.offset + i];
    lightDataSource[i].lightID = lightID;
    EnvironmentLight envLight = environmentLights[lightID];
    lightDataSource[i].arrayIndex = envLight.arrayIndex;
    const float r = envLight.sphereRange;
    const float r2 = r*r;
    
    
    // volume
    lightDataSource[i].volume = 4.0 / 3.0 * PI * r2*r;
    
    //influence
    vec3 diff  = envLight.position.xyz - positionWorld;
    float squaredDistance = dot(diff, diff);
    
    // if squaredDistance <= r*r
    lightDataSource[i].influence = 1.0 - smoothstep(0.8 * r2, r2, squaredDistance);
  }
  
  EnvLightData tmp;
  
  // sort by influence
  CSWAP_INFLUENCE(lightDataSource, 0, 1, tmp);
  CSWAP_INFLUENCE(lightDataSource, 2, 3, tmp);
  CSWAP_INFLUENCE(lightDataSource, 0, 2, tmp);
  CSWAP_INFLUENCE(lightDataSource, 1, 3, tmp);
  CSWAP_INFLUENCE(lightDataSource, 1, 2, tmp);
  
  // sort by volume
  CSWAP_VOLUME(lightDataSource, 0, 1, tmp);
  CSWAP_VOLUME(lightDataSource, 2, 3, tmp);
  CSWAP_VOLUME(lightDataSource, 0, 2, tmp);
  CSWAP_VOLUME(lightDataSource, 1, 3, tmp);
  CSWAP_VOLUME(lightDataSource, 1, 2, tmp);
  
  
  
  ArrayIndexWeight result;
  result.indices[0] = 0;
  result.indices[1] = 0;
  result.weights[0] = 0.0;
  result.weights[1] = 0.0;
  
  float summedWeights = 0.0;
  
  for (int i = 0; i < lightGrid.count; ++i) {
    result.indices[i] = lightDataSource[i].arrayIndex;
    result.weights[i] = lightDataSource[i].influence;    
    
    summedWeights = clamp(summedWeights + result.weights[i], 0.0, 1.0);
  }
  
  
  return result;
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