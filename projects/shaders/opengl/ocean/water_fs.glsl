#version 460 core

#ifndef CSM_CASCADE_DEPTH_MAP_BINDING_POINT
#define CSM_CASCADE_DEPTH_MAP_BINDING_POINT 8
#endif

#ifndef PPR_CLEAR_VALUE
#define PPR_CLEAR_VALUE 0x0
#endif

#include "interface/buffers.h"
#include "shadow/cascaded_shadow.glsl"


const float PI = 3.14159265359;

in VS_OUT {
    vec3 normal;
    vec3 normalWorld;
    vec3 positionView;
    vec3 waterSurfaceZeroView;
    vec3 positionWorld;
    vec4 positionCS;
    vec2 texCoords;
	vec2 slopeXZ;
} vs_out;

//in vec2 texCoord_tcs_in;

layout(location = 0)out vec4 fragColor;
layout(location = 1)out vec4 luminance;
layout(location = 2)out vec2 motion;
//layout(location = 3)out float depth;



layout(binding = 0) uniform sampler2D height;
layout(binding = 1) uniform sampler2D slopeX;
layout(binding = 2) uniform sampler2D slopeZ;
layout(binding = 3) uniform sampler2D dX;
layout(binding = 4) uniform sampler2D dZ;

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

layout(binding = 10) uniform sampler3D voxelTexture;

layout(binding = 11) uniform sampler2D foamMap;
layout(binding = 12) uniform usampler2D projHashMap;

uniform float waterLevel;
uniform int usePSSR;

#include "GI/cone_trace.glsl"


float getLuma(in vec3 rgb) {
    return 1.0 * rgb.r + 1.0 * rgb.g + 1.0 * rgb.b; 
}


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

float calcFoam(in vec3 waterPositionWorld, in vec3 groundPositionWorld, in vec3 eyeVecNorm) {

    // Describes at what depth foam starts to fade out and
    // at what it is completely invisible. The fird value is at
    // what height foam for waves appear (+ waterLevel).
    const vec3 foamExistence = vec3(0.3, 0.8, 0.25);//vec3(0.65, 1.35, 0.5);
    const float maxAmplitude = 1.0;
    
    
    const float diffY = waterPositionWorld.y - groundPositionWorld.y;
    const float level = waterPositionWorld.y;
    float testWaterLevel = level;
	
    //animationTime * 0.00001 * windDirection
    vec2 texCoord = (waterPositionWorld.xz + eyeVecNorm.xz * 0.00) * 0.05 + animationTime * 0.003 * windDirection;
    //vec2 texCoord = (waterPositionWorld.xz) * 0.05 + animationTime * 0.00001 * windDirection + sin(animationTime * 0.001 + groundPositionWorld.x) * 0.005;
    
	vec2 texCoord2 = (waterPositionWorld.xz + eyeVecNorm.xz * 0.00) * 0.05 + animationTime * 0.006 * windDirection;
    //vec2 texCoord2 = (waterPositionWorld.xz) * 0.05 + animationTime * 0.002 * windDirection + sin(animationTime * 0.01 + groundPositionWorld.z) * 0.05;


	const float slope = pow((vs_out.slopeXZ.x + vs_out.slopeXZ.y), 0.7);

    float foam = 0.0;
    
    if (diffY < foamExistence.x) {
        foam = slope * (texture(foamMap, texCoord).r + texture(foamMap, texCoord2).r);
    } else if (diffY < foamExistence.y) {
        foam = mix(slope * (texture(foamMap, texCoord).r + texture(foamMap, texCoord2).r), 0.0,
					 (diffY - foamExistence.x) / (foamExistence.y - foamExistence.x));
    }
    
    if(maxAmplitude - foamExistence.z > 0.0001f)
    {
        foam += (texture(foamMap, texCoord).r + texture(foamMap, texCoord2).r) * 
            clamp(3.0 * (level - (testWaterLevel + foamExistence.z)) /(maxAmplitude - foamExistence.z), 0.0, 1.0);
    }
	
	
	foam += 0.1 * slope;
    
    

//windDirection
//animationTime

    foam = clamp(foam, 0, 1);

    return pow(0.2 * foam, 1.3);
}


vec2 decodeHash(uint hash, in vec2 texSize) {
    uint x = hash & 0xFFFF; // first 16 bits
    int y = int(hash >> 16); //remove first 16 bits
    y = -(y - int(texSize.y + 1000));
   return  vec2(float(x) / texSize.x, float(y) / texSize.y);
}


/**
 * Provides the uv coordinates of the reflected color.
 */
vec4 resolveHash(in vec2 uv, in vec3 positionWS) 
{
    const bool enableHolesFilling = true;
    


    vec2 texSize = vec2(textureSize(depthMap, 0));
    uint hash = texture(projHashMap, uv).r;
    vec2 sourceUV = decodeHash(hash, texSize);
    ivec2 offset = ivec2(0,0);
    
    uint hashMin = hash;
    
    if (enableHolesFilling) {
    
        const int scale = 1;
        
        for (int i = 1; i <= 1; ++i ) {
            const vec2 holeOffset1 = vec2(i,0)/texSize;
            const vec2 holeOffset2 = vec2(0,i)/texSize;
            const vec2 holeOffset3 = vec2(i,i)/texSize;
            const vec2 holeOffset4 = vec2(-i,0)/texSize;
            
            uint hash1 = texture(projHashMap, uv + holeOffset1).r;
            uint hash2 = texture(projHashMap, uv + holeOffset2).r;
            uint hash3 = texture(projHashMap, uv + holeOffset3).r;
            uint hash4 = texture(projHashMap, uv + holeOffset4).r;
            
            hashMin = max(max(max(hashMin, hash1), max(hash2, hash3)), hash4);
            
            if (hashMin != 0) break;
        }
        
        
        
        
        
        // Allow hole fill if we don't have any valid hash data for the current pixel,
        // or any neighbor has reflection significantly closer than current pixel's reflection
        
        bool allowHoleFill = true;
        if (hash == PPR_CLEAR_VALUE) {
            vec2 minUV = decodeHash(hashMin, texSize);
            vec2 sourceOffset = sourceUV - uv;
            vec2 minOffset = minUV - uv;
            vec2 diffTexel = (sourceOffset - minOffset) * texSize;
            const float minDist = 6.0f;
            allowHoleFill = dot(diffTexel, diffTexel) > minDist * minDist;
        }
        
        if (allowHoleFill && hashMin != 0) {
            sourceUV = decodeHash(hashMin, texSize);
            //if (hashMin == hash1) offset = holeOffset1;
            //if (hashMin == hash2) offset = holeOffset2;
            //if (hashMin == hash3) offset = holeOffset3;
            //if (hashMin == hash4) offset = holeOffset4;
        }
    
    }
    
    
    
    
    if (hash == PPR_CLEAR_VALUE && (hashMin == PPR_CLEAR_VALUE)) return vec4(0.0);
   
    //sourceUV += vec2(offset) / texSize;
    
    
    return texture(colorMap, sourceUV);
}


// ----------------------------------------------------------------------------
float fresnelSchlick(in float cosTheta, in float F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
float fresnelSchlickRoughness(in float cosTheta, in float F0, in float roughness)
{
    return F0 + (max(1.0 - roughness, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   
// ----------------------------------------------------------------------------

void main() {

    vec3 normal = normalize(vs_out.normal);
    vec3 lightDir = normalize(-lightDirViewSpace);
    vec3 eyeVecNorm = normalize(-vs_out.positionView.xyz);
    vec3 halfway = normalize(lightDir + eyeVecNorm);
    float angle = max(dot(normal, lightDir), 0.0);
    float fragmentLitProportion = cascadedShadow(lightDir, normal, vs_out.positionView.z, vs_out.positionView);
    const float refractionRatio = 1.0 / 1.3;
     // vec4(0.0, 0.0, 1.0, 1.0);
    
  
    // uvs
    vec2 uv = vs_out.positionCS.xy;// / vs_out.positionCS.w;
    vec2 refractionUV = vs_out.positionCS.xy;
    refractionUV.x += normal.x * refractionRatio * 0.03 / vs_out.positionCS.w;
    refractionUV.y += normal.z * refractionRatio * 0.03 / vs_out.positionCS.w;
    //refractionUV.xy = clamp(refractionUV.xy, 0.0, 1.0);
    refractionUV= refractionUV.xy;// / vs_out.positionCS.w;
    vec2 refractionUVUnclamped = refractionUV;
    refractionUV = clamp(refractionUV, 0.001, 0.999);
    
    
    
    
 
    
    
    //water depths;
    float refractionDepth = texture(depthMap, refractionUV).r;
    float nonRefractionDepth = texture(depthMap, uv).r;
    float waterHeightDepth = gl_FragCoord.z;
    
    
    //positions
    vec3 positionWorld = reconstructPositionFromDepth(inverseViewProjMatrix, uv, waterHeightDepth);
    vec3 refractionPositionWorld = reconstructPositionFromDepth(inverseViewProjMatrix, refractionUV, refractionDepth);
    vec3 nonRefractionPositionWorld = reconstructPositionFromDepth(inverseViewProjMatrix, uv, nonRefractionDepth);
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
    vec3 extinction = vec3(4.5, 35.0, 50.0); // 4.5 , 75, 300
    float D = waterY - refractionY;
    float murk = 0.5;
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
                        calcSpecular(eyeVecNorm, normal, normalize(lightDirViewSpace), 0.2, 0);
    
    
    float foam = calcFoam(positionWorld, nonRefractionPositionWorld, eyeVecNorm);
    
    
    vec3 diffuseRefraction = refractionColor * angle * fragmentLitProportion * 0.15;
    vec3 ambientRefraction = refractionColor * irradianceLuma;
    
    float lit = max(irradianceLuma, fragmentLitProportion);
    float litSpecular = max(0, fragmentLitProportion);
    
    vec3 specular = litSpecular * specular_color;
    
    
    
    fragColor = vec4(diffuseRefraction + ambientRefraction + 0.1 * specular + vec3(lit * foam), 1.0);
    
    
	
	// planar screen space reflections
	if (usePSSR > 0 && false) {
		vec4 pssrColor = resolveHash(refractionUV, positionWorld);
		if (pssrColor.a > 0) {
			float viewAngle = max(dot(eyeVecNorm, vec3(0, 1, 0)), 0.0);
			float reflectivity = fresnelSchlick(viewAngle, 0.0);
			pssrColor.rgb = mix(pssrColor.rgb, fragColor.rgb, 0.7);
			//float reflectivity = pow(1.0 - viewAngle, 5.0);
			fragColor.rgb = mix(pssrColor.rgb, fragColor.rgb, reflectivity);
			//fragColor.rgb = mix(pssrColor.rgb, fragColor.rgb, 0.0);
		}
	}
    
    
    //fragColor = vec4(1,0,1,1);  
    luminance = 0.1 * fragColor;//texture(luminanceMap, refractionUV);
	motion = vec2(0.0);
}