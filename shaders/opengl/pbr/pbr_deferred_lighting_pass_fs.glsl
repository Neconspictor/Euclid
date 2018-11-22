#version 430
#extension GL_EXT_texture_array : enable

#define NUM_CASCADES 4

out vec4 FragColor;

const float PI = 3.14159265359;

struct GBuffer {
    sampler2D albedoMap;
	sampler2D aoMetalRoughnessMap;
	sampler2D normalEyeMap;
	sampler2D positionEyeMap;
};


struct DirLight {
    vec3 directionEye;
    vec3 color;
};

struct CascadeData {
	//mat4 viewMatrix;
	mat4 inverseViewMatrix;
	mat4 lightViewProjectionMatrices[NUM_CASCADES];
	vec4 cascadedSplits[NUM_CASCADES];
};


in VS_OUT {	
	vec2 tex_coords;
} fs_in;

uniform DirLight dirLight;

uniform GBuffer gBuffer;
uniform sampler2D shadowMap;
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

uniform sampler2DArrayShadow cascadedDepthMap;



/*******************************************************************/
 
 
#define BLOCKER_SEARCH_NUM_SAMPLES 16 
#define PCF_NUM_SAMPLES 36
#define NEAR_PLANE 0.0f 
#define LIGHT_WORLD_SIZE 0.1f 
#define LIGHT_FRUSTUM_WIDTH 3.75f 

// Assuming that LIGHT_FRUSTUM_WIDTH == LIGHT_FRUSTUM_HEIGHT 
#define LIGHT_SIZE_UV (LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH) 


vec2 poissonDisk[16] = { 
	vec2 ( -0.94201624, -0.39906216 ), 
	vec2 ( 0.94558609, -0.76890725 ), 
	vec2 ( -0.094184101, -0.92938870 ), 
	vec2 ( 0.34495938, 0.29387760 ), 
	vec2 ( -0.91588581, 0.45771432 ), 
	vec2 ( -0.81544232, -0.87912464 ), 
	vec2 ( -0.38277543, 0.27676845 ), 
	vec2 ( 0.97484398, 0.75648379 ), 
	vec2 ( 0.44323325, -0.97511554 ), 
	vec2 ( 0.53742981, -0.47373420 ), 
	vec2 ( -0.26496911, -0.41893023 ), 
	vec2 ( 0.79197514, 0.19090188 ), 
	vec2 ( -0.24188840, 0.99706507 ), 
	vec2 ( -0.81409955, 0.91437590 ), 
	vec2 ( 0.19984126, 0.78641367 ), 
	vec2 ( 0.14383161, -0.14100790 )
};
 
 
struct BlockerResult {
	float avgBlockerDepth;
	float numBlockers;
}; 
 
 
 
 
/*******************************************************************/


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
float shadowCalculation(sampler2D shadowMap, vec3 lightDir, vec3 normal, vec4 fragment_position_lightspace);
float shadowCalculationVariance(vec3 lightDir, vec3 normal, vec4 fragment_position_lightspace);
float texture2DArrayCompare(sampler2DArray depths, vec3 uv, float compare, float bias);
float texture2DCompare(sampler2D depths, vec2 uv, float compare, float bias);
float texture2DArrayShadowLerp(sampler2DArrayShadow depths, vec2 size, vec2 uv, float projectedZ, float compare, float bias);
float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float PCF(sampler2D depths, vec2 size, vec2 uv, float compare, float bias, float penumbraSize);
float chebyshevUpperBound( float distance, vec2 uv);

float PCSS (sampler2D shadowMap, vec3 coords, float bias);
float PCF_Filter(sampler2D shadowMap, vec2 uv, float zReceiver, float filterRadiusUV, float bias);




void main()
{   
	vec3 albedo = texture(gBuffer.albedoMap, fs_in.tex_coords).rgb;
	
	vec3 aoMetalRoughness = texture(gBuffer.aoMetalRoughnessMap, fs_in.tex_coords).rgb;
	float ao = aoMetalRoughness.r;
	float metallic = aoMetalRoughness.g;
	float roughness = aoMetalRoughness.b;
	
	vec3 normalEye = normalize(texture(gBuffer.normalEyeMap, fs_in.tex_coords).rgb);
	/*float alpha = length(normalEye);
	if (alpha < 0.9) {
		discard;
	};*/
	
	
	vec3 positionEye = texture(gBuffer.positionEyeMap, fs_in.tex_coords).rgb;
	
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
	
	shadow = clamp(shadow, 0.2, 1);
	
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

    return (kD * diffuse + ambientLightSpecular * 0.4) * ao;
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
	
	float positiveViewSpaceZ = depthViewSpace;
	uint cascadeIdx = 0;

	// Figure out which cascade to sample from
	for(uint i = 0; i < NUM_CASCADES - 1; ++i)
	{
		if(positiveViewSpaceZ < cascadeData.cascadedSplits[i].x)
		{	
			cascadeIdx = i + 1;
		}
	}
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
	bias =  9 * minBias;

	float shadow = 0.0;
	//vec2 texelSize = 1.0 / textureSize(cascadedDepthMap, 0).xy;
	
	float xSamples = 3;
	float ySamples = 3;
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
			//off = vec2(0,0);
			//result += texture2DCompare(depths, uv, compare, bias);
            //result += texture2DShadowLerp(depths, size, uv+off, compare, bias);
			vec2 uv = projCoords.xy + off;
			float compare = currentDepth - bias;
			//float pcfDepth =  texture2DArrayShadowLerp(cascadedDepthMap, size, uv, projCoords.z, currentDepth, bias);
			float pcfDepth =  shadow2DArray(cascadedDepthMap, vec4(uv, projCoords.z, currentDepth - bias )).r; 
			//shadow += pcfDepth;
			shadow += currentDepth  > pcfDepth ? 0.0  : 1.0;
        }
    }
    return shadow / (sampleCount);

	//float pcfDepth =  shadow2DArray(cascadedDepthMap, vec4(projCoords.xyz, currentDepth + bias )).r; 
	//shadow += currentDepth  > pcfDepth ? 0.0  : 1.0;
	//return shadow; 
}




float shadowCalculation(sampler2D shadowMap, vec3 lightDir, vec3 normal, vec4 fragment_position_lightspace)
{		
	vec3 shadowCoordinateWdivide = fragment_position_lightspace.xyz / fragment_position_lightspace.w;
	shadowCoordinateWdivide.r = shadowCoordinateWdivide.r*0.5 + 0.5;
	shadowCoordinateWdivide.g = shadowCoordinateWdivide.g*0.5 + 0.5;
	
	float currentDepth = shadowCoordinateWdivide.z*0.5 + 0.5;
	float angle = dot(lightDir, normal);
	vec2 texelSize = vec2(1.0)/textureSize(shadowMap, 0);
	
	float sDotN = dot(lightDir, normal);
	
	float sDotNAbs = abs(sDotN);
	
	// assure that fragments with a normal facing away from the light source 
	// are always in shadow (reduces unwanted unshadowing).
	if (sDotN < 0) {
		return 0;
	}
	
	if (sDotNAbs < (0.3f)) {
		//return 0;
	}
	
	float bias = 0.0;
	float shadow;
	float depth = texture2D(shadowMap, shadowCoordinateWdivide.xy).r;
	float diff =  abs(currentDepth - depth);

	

	//bias = 9.0f * max(texelSize.x,texelSize.y);
	float minBias = max(texelSize.x,texelSize.y);
	float penumbraSize = diff / (minBias);
	penumbraSize = clamp(penumbraSize, 0, 1);
	//penumbraSize = max(penumbraSize, 0.0f);
	//penumbraSize =  min(penumbraSize, 1);
	penumbraSize = 1;
	bias = 2 * minBias;
	
	shadow = PCF(shadowMap, textureSize(shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias, penumbraSize);
	//shadow = texture2DCompare(shadowMap, shadowCoordinateWdivide.xy, currentDepth, bias);
	//shadow = PCSS(material.shadowMap, shadowCoordinateWdivide, bias);
	//shadow = PCF_Filter(material.shadowMap, shadowCoordinateWdivide.xy, currentDepth, 0.005f, bias);
	
	
	//float shadow = textureProj(material.shadowMap, test, 0);
	//float shadow = texture2DCompare(material.shadowMap, shadowCoordinateWdivide.xy, currentDepth, bias);
	//float shadow = PCF(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);
	//float shadow = texture2DShadowLerp(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);
    return 1 - shadow;	
}

float shadowCalculationVariance(vec3 lightDir, vec3 normal, vec4 fragment_position_lightspace) {
		if (fragment_position_lightspace.z >= 1.0)
				return 0.0;
	
		return 1 - chebyshevUpperBound(fragment_position_lightspace.z, fragment_position_lightspace.xy);
}

float texture2DArrayCompare(sampler2DArray depths, vec3 uv, float compare, float bias){
    float depth = texture2DArray(depths, uv).r;
		if (depth >= 1.0)
			return 0.0f;
    return step(depth + bias, compare);
}

float texture2DCompare(sampler2D depths, vec2 uv, float compare, float bias){
    float depth = texture2D(depths, uv).r;
		if (depth >= 1.0)
			return 0.0f;
    return step(depth + bias, compare);
}

float texture2DArrayShadowLerp(sampler2DArrayShadow depths, vec2 size, vec2 uv, float projectedZ, float compare, float bias){
    vec2 texelSize = vec2(1.0)/size;
    vec2 f = fract(uv*size+0.5);
    vec2 centroidUV = (uv*size+0.5)/size;

    float lb = shadow2DArray(depths, vec4(centroidUV +texelSize*vec2(0.0, 0.0), projectedZ, compare - bias)).r;
	float lt = shadow2DArray(depths, vec4(centroidUV +texelSize*vec2(0.0, 1.0), projectedZ, compare - bias)).r;
	float rb = shadow2DArray(depths, vec4(centroidUV +texelSize*vec2(1.0, 0.0), projectedZ, compare - bias)).r;
	float rt = shadow2DArray(depths, vec4(centroidUV +texelSize*vec2(1.0, 1.0), projectedZ, compare - bias)).r;
	float a = mix(lb, lt, f.y);
	float b = mix(rb, rt, f.y);
    float c = mix(a, b, f.x);
    return c;
}

float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare, float bias){
    vec2 texelSize = vec2(1.0)/size;
    vec2 f = fract(uv*size+0.5);
    vec2 centroidUV = (uv*size+0.5)/size;

    float lb = texture2DCompare(depths, centroidUV+texelSize*vec2(0.0, 0.0), compare, bias);
    float lt = texture2DCompare(depths, centroidUV+texelSize*vec2(0.0, 1.0), compare, bias);
    float rb = texture2DCompare(depths, centroidUV+texelSize*vec2(1.0, 0.0), compare, bias);
    float rt = texture2DCompare(depths, centroidUV+texelSize*vec2(1.0, 1.0), compare, bias);
    float a = mix(lb, lt, f.y);
    float b = mix(rb, rt, f.y);
    float c = mix(a, b, f.x);
    return c;
}

float PCF(sampler2D depths, vec2 size, vec2 uv, float compare, float bias, float penumbraSize){
    float result = 0.0;
		float xSamples = 2;
		float ySamples = 2;
		float sampleCount = (2*xSamples + 1) * (2*ySamples + 1);
		
    for(float x=-xSamples; x<=xSamples; x += 1){
        for(float y=-ySamples; y<=ySamples; y += 1){
            vec2 off = vec2(x,y)/size * penumbraSize;
			//off = vec2(0,0);
			//result += texture2DCompare(depths, uv, compare, bias);
            result += texture2DShadowLerp(depths, size, uv+off, compare, bias);
        }
    }
    return result / (sampleCount);
}




/***************************************************************************************************/


float PenumbraSize(float zReceiver, float zBlocker) { //Parallel plane estimation
return (zReceiver - zBlocker) / zBlocker; 
} 

BlockerResult FindBlocker(sampler2D shadowMap, vec2 uv, float zReceiver ) {

	BlockerResult result;


    //This uses similar triangles to compute what  
    //area of the shadow map we should search 
    float searchWidth = LIGHT_SIZE_UV * (zReceiver - NEAR_PLANE) / zReceiver; 
    float blockerSum = 0; 
    result.numBlockers = 0; 
    
	for( int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i ) { 
		float shadowMapDepth = texture2D(shadowMap, uv + poissonDisk[i] * searchWidth).r; 
		
		if ( shadowMapDepth < zReceiver ) { 
                blockerSum += shadowMapDepth; 
                result.numBlockers++; 
            } 
     }
	 
    result.avgBlockerDepth = blockerSum / result.numBlockers; 
	
	return result;
}
 
float PCF_Filter(sampler2D shadowMap, vec2 uv, float zReceiver, float filterRadiusUV, float bias) { 
    
	vec2 size = textureSize(shadowMap, 0);
	
	float sum = 0.0f; 
	
	
    for ( int i = 0; i < PCF_NUM_SAMPLES; ++i ) { 
		vec2 offset = poissonDisk[i] * filterRadiusUV; 
        sum += texture2DShadowLerp(shadowMap, size, uv + offset, zReceiver, bias);
    } 
	
    return sum / PCF_NUM_SAMPLES; 
} 

float PCSS (sampler2D shadowMap, vec3 coords, float bias) 
{ 
    vec2 uv = coords.xy; 
    float zReceiver = coords.z; 
	
	// Assumed to be eye-space z in this code
    // STEP 1: blocker search 

    BlockerResult blockerResult = FindBlocker(shadowMap, uv, zReceiver ); 
	
    if( blockerResult.numBlockers == 0.0f )   
		//There are no occluders so early out (this saves filtering) 
		return 1.0f; 
	
    // STEP 2: penumbra size 
    float penumbraRatio = PenumbraSize(zReceiver, blockerResult.avgBlockerDepth);     
    float filterRadiusUV = penumbraRatio * LIGHT_SIZE_UV /* NEAR_PLANE*/ / coords.z; 
	filterRadiusUV *= 0.5;
	//filterRadiusUV = 1.0f;
	
	
	
    // STEP 3: filtering 
	//return PCF(shadowMap, textureSize(shadowMap, 0), uv, zReceiver, bias);
    return PCF_Filter(shadowMap, uv, zReceiver, filterRadiusUV, bias); 
}


/*************************************************************************************************/