#version 450

#ifndef CSM_CASCADE_BUFFER_BINDING_POINT
#define CSM_CASCADE_BUFFER_BINDING_POINT 0
#endif

#ifndef CSM_CASCADE_DEPTH_MAP_BINDING_POINT
#define CSM_CASCADE_DEPTH_MAP_BINDING_POINT 8
#endif

const float PI = 3.14159265359;

in VS_OUT {
    vec3 normal;
    vec3 normalWorld;
    vec3 positionView;
    vec3 waterSurfaceZeroView;
    vec3 positionWorld;
    vec4 positionCS;
    vec2 texCoords;
} vs_out;

//in vec2 texCoord_tcs_in;

layout(location = 0)out vec4 fragColor;
layout(location = 1)out vec4 luminance;
//layout(location = 2)out vec2 motion;
//layout(location = 3)out float depth;

layout(binding = 5) uniform sampler2D colorMap;
layout(binding = 6) uniform sampler2D luminanceMap;
layout(binding = 7) uniform sampler2D depthMap;

//layout(binding = 7) uniform sampler2D reflectionMap;


uniform vec3 lightDirViewSpace;
uniform mat3 normalMatrix;
uniform mat4 inverseViewProjMatrix;
uniform vec3 cameraPosition;

uniform vec2 windDirection;
uniform float animationTime;

#include "shadow/cascaded_shadow.glsl"

layout(binding = 9) uniform sampler2D irradianceMap;

layout(binding = 10) uniform sampler2D foamMap;

/**
 * Cone tracing
 */
layout(std140, binding = 0) uniform Cbuffer {
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

layout(binding = 10) uniform sampler3D voxelTexture;

#include "GI/cone_trace.glsl"


float getLuma(in vec3 rgb) {
    return 1.0 * rgb.r + 1.0 * rgb.g + 1.0 * rgb.b; 
}


vec3 computeWorldPositionFromDepth(in vec2 texCoord, in float depth) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depth * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = inverseViewProjMatrix * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};


vec4 calcAmbientColor(in vec3 normal, in vec3 halfway, in float angle) 
{
    vec4 c = vec4(1,1,1,1);
    float fog_factor = 0.0;

	vec4 emissive_color = vec4(1.0, 1.0, 1.0,  1.0);
	vec4 ambient_color  = vec4(0.0, 0.65, 0.75, 1.0);
	vec4 diffuse_color  = vec4(0.5, 0.65, 0.75, 1.0);
	vec4 specular_color = vec4(0.1, 0.5, 1.0,  1.0);

	float emissive_contribution = 0.00;
	float ambient_contribution  = 0.30;
	float diffuse_contribution  = 0.3;
	float specular_contribution = 1.80;

	bool facing = angle > 0.0;

	vec4 colorOut = emissive_color * emissive_contribution +
		    ambient_color  * ambient_contribution  * c +
		    diffuse_color  * diffuse_contribution  * c * angle +
                    (facing ?
			specular_color * specular_contribution * c * max(pow(dot(normal, halfway), 70.0), 0.0) :
			vec4(0.0, 0.0, 0.0, 0.0));

	return colorOut * (1.0-fog_factor) + vec4(0.25, 0.75, 0.65, 1.0) * (fog_factor);
}

float calcMurkiness(in float waterY, in float groundY, in float maxY) {
    float murk = waterY - groundY;
    
    murk = clamp(murk, 0.0, maxY);
    
    // normalize Note: if maxY is zero we have to clamp!    
    murk = clamp(murk / maxY, 0.0, 1.0);
    
    return murk;
}

float calcSpecular(in vec3 eyeVecNorm, in vec3 normal, in vec3 lightDir, float shininess, float fresnel) 
{
    vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
    float dotSpec = clamp(dot(mirrorEye.xyz, -lightDir) * 0.5 + 0.5, 0, 1);
    float specular = (1.0 - fresnel) * clamp(-lightDir.y, 0, 1) * ((pow(dotSpec, 512.0)) * (shininess * 1.8 + 0.2));
    specular += specular * 25 * clamp(shininess - 0.05, 0, 1);
    
    return specular;
    
    /*vec3 halfway = normalize(-lightDir + eyeVecNorm);
    float angle = max(dot(normal, -lightDir), 0.0);
    float specular_contribution = 1.80;
    bool facing = angle > 0.0;
    return facing ? specular_contribution * max(pow(dot(normal, halfway), 512.0), 0.0) : 0.0;
    */
}

float calcFoam(in vec3 waterPositionWorld, in vec3 groundPositionWorld) {

    // Describes at what depth foam starts to fade out and
    // at what it is completely invisible. The fird value is at
    // what height foam for waves appear (+ waterLevel).
    const vec3 foamExistence = vec3(0.65, 1.35, 0.5);
    
    const float diffY = waterPositionWorld.y - groundPositionWorld.y;
    
    float foam = 0.0;
    
    /*if (diffY < foamExistence.x) {
        foam = (texture(foamMap, texCoord).r + texture(foamMap, texCoord2).r) * 0.5;
    } else if (diffY < foamExistence.y) {
        foam = mix((texture(foamMap, texCoord).r + texture(foamMap, texCoord2).r) * 0.5, 0.0,
						 (diffY - foamExistence.x) / (foamExistence.y - foamExistence.x));
    }*/
    
    

//windDirection
//animationTime

    return foam;
}


void main() {

    vec3 normal = normalize(vs_out.normal);
    vec3 lightDir = normalize(-lightDirViewSpace);
    vec3 halfway = normalize(lightDir + normalize(-vs_out.positionView.xyz));
    float angle = max(dot(normal, lightDir), 0.0);
    float fragmentLitProportion = cascadedShadow(lightDir, normal, vs_out.positionView.z, vs_out.positionView);
    const float refractionRatio = 1.0 / 1.3;
     // vec4(0.0, 0.0, 1.0, 1.0);
    
  
    // uvs
    vec2 uv = vs_out.positionCS.xy / vs_out.positionCS.w;
    vec2 refractionUV = vs_out.positionCS.xy;
    refractionUV.x += normal.x * refractionRatio * 0.05;
    refractionUV.y += normal.z * refractionRatio * 0.05;
    //refractionUV.xy = clamp(refractionUV.xy, 0.0, 1.0);
    refractionUV= refractionUV.xy / vs_out.positionCS.w;
    
    refractionUV = clamp(refractionUV, 0.001, 0.999);
    
    
    
    
 
    
    
    //water depths;
    float refractionDepth = texture(depthMap, refractionUV).r;
    float nonRefractionDepth = texture(depthMap, uv).r;
    float waterHeightDepth = gl_FragCoord.z;
    
    
    //positions
    vec3 positionWorld = computeWorldPositionFromDepth(uv, waterHeightDepth);
    vec3 refractionPositionWorld = computeWorldPositionFromDepth(refractionUV, refractionDepth);
    vec3 nonRefractionPositionWorld = computeWorldPositionFromDepth(uv, nonRefractionDepth);
    vec3 testWorld = vec3(refractionPositionWorld.x,positionWorld.y , refractionPositionWorld.z);
    
    //y coords
    float refractionY = refractionPositionWorld.y;
    float waterY = positionWorld.y;
    
    // colors
    vec3 nonRefractedColor = texture(colorMap, uv).rgb;
    vec3 refractionColor = texture(colorMap, refractionUV).rgb;
    //vec4 irradianceColor = texture(irradianceMap, uv);
    
    vec4 coneTracedIrradiance = ConeTraceRadiance(positionWorld, vs_out.normalWorld);
    vec3 irradiance = coneTracedIrradiance.a * coneTracedIrradiance.rgb;
    
    float irradianceLuma = max(clamp(1.0 * getLuma(irradiance.rgb),0, 1), 0.05);
    
    //calculate murkiness
    //float murk = calcMurkiness(waterY, refractionY, 2.0);
    //float murkLit = max(fragmentLitProportion, irradianceLuma);
    //vec4 ambient = calcAmbientColor(normal, halfway, angle);
    //vec4 murkColor = ambient;
    
  
  
    //illumination
    //float litLuma = clamp(getLuma(refractionColor.rgb), 0.0, 1.0);
    
    // final color output
    //fragColor = refractionColor;
    //fragColor.a = 1.0;
    //fragColor = mix(fragColor, vec4(1.0, 1.0, 1.0, fragColor.a), 0.1);
    //fragColor.rgb *= litLuma * vec3(1.0, 0.9, 0.7);
    
    
    //fragColor.rgb = mix(litLuma * refractionColor.rgb, murkColor.rgb, murk);
    //fragColor.rgb = max(fragmentLitProportion, irradianceLuma) * mix(fragColor.rgb, murkColor.rgb, murk);
    
    
    // realsitic water 
    vec3 extinction = vec3(4.5, 75.0, 300.0);
    float D = waterY - refractionY;
    float murk = 0.25;
    float A = length(vs_out.positionWorld - refractionPositionWorld) * murk;// * pow(murk, 2);
    vec3 surfaceColor = vec3(0.0078, 0.2176, 0.7);//vec3(0.0078, 0.5176, 0.7);//ambient.rgb;//vec3(1.0, 1.0, 1.0);
    vec3 bigDepthColor = vec3(0.0039, 0.00196, 0.145);
    vec3 testCameraPosition = vec3(0, 12.9, -0.5);
    float distanceToCamera = length(positionWorld - cameraPosition); 
    //distanceToCamera = 9;
    float fadeDistance = 120;
    float visibility = 3.0;//clamp((fadeDistance - distanceToCamera) / fadeDistance, 0, 1); // lesser for positions farer away from camera  1 / distanceToCamera
    //visibility = clamp(pow(visibility, 7), 0, 1);
    vec3 sunColor = vec3(1.0, 0.89, 0.4);//vec3(1.0);
    float sunScale = 3.0;

    
    vec3 waterColor = vec3(clamp(length(sunColor) / sunScale, 0, 1));
    
    refractionColor = mix(refractionColor, surfaceColor * waterColor, clamp( A / visibility, 0.0, 1.0));
    refractionColor = mix(refractionColor, bigDepthColor * waterColor, clamp(D / extinction, 0.0, 1.0));
    
    
    vec3 specular_color = sunColor * 
                        calcSpecular(normalize(-vs_out.positionView.xyz), normal, normalize(lightDirViewSpace), 0.2, 0);
    
    
    float foam = calcFoam(positionWorld, nonRefractionPositionWorld);
    
    
    vec3 diffuseRefraction = refractionColor * angle * fragmentLitProportion * 0.15;
    vec3 ambientRefraction = refractionColor * irradianceLuma;
    
    float lit = max(irradianceLuma, fragmentLitProportion);
    float litSpecular = max(0, fragmentLitProportion);
    
    fragColor = vec4(diffuseRefraction + ambientRefraction + litSpecular * specular_color, 1.0);
      
    luminance = 0.1 * fragColor;//texture(luminanceMap, refractionUV);
}