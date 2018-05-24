#version 400


const float PI = 3.14159265359;

struct Material {
    sampler2D albedoMap;
	sampler2D aoMap;
	sampler2D metallicMap;
	sampler2D normalMap;
	sampler2D roughnessMap;
	sampler2D shadowMap;
};


struct DirLight {
    vec3 direction;
    vec3 color;
};


in VS_OUT {	
	vec3 fragment_position_eye;
	vec2 tex_coords;
	vec3 normal_view;
	vec4 fragment_position_lightspace; // needed for shadow calculation
	mat3 TBN_eye_directions; // used to transform the normal vector from tangent to eye space.
						  //  This matrix mustn't be used with positions!!!
	vec3 light_direction_eye; // the light direction in tangent space	
} fs_in;


out vec4 FragColor;

uniform DirLight dirLight;

uniform Material material;

uniform mat4 model;
uniform mat4 modelView;
uniform mat4 inverseView;


// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;


uniform vec3 cameraPos;

uniform mat4 view;



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









vec3 pbrModel(vec3 normalEye, vec3 viewDirectionEye, vec3 lightDirectionEye);
vec3 pbrDirectLight(vec3 V, vec3 N, vec3 L, float roughness, vec3 F0, float metallic, vec3 albedo);
vec3 pbrAmbientLight(vec3 V, vec3 N, float roughness, vec3 F0, float metallic, vec3 albedo, vec3 R, float ao);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);


float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragment_position_lightspace);
float shadowCalculationVariance(vec3 lightDir, vec3 normal, vec4 fragment_position_lightspace);
float texture2DCompare(sampler2D depths, vec2 uv, float compare, float bias);
float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float PCF(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float chebyshevUpperBound( float distance, vec2 uv);

float PCSS (sampler2D shadowMap, vec3 coords, float bias);
float PCF_Filter(sampler2D shadowMap, vec2 uv, float zReceiver, float filterRadiusUV, float bias);



// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
	float factor = 255/128.0f;
	
	vec3 tangentNormal = (texture(material.normalMap, fs_in.tex_coords).xyz * factor) - 1.0;

    vec3 Q1  = dFdx(fs_in.fragment_position_eye);
    vec3 Q2  = dFdy(fs_in.fragment_position_eye);
    vec2 st1 = dFdx(fs_in.tex_coords);
    vec2 st2 = dFdy(fs_in.tex_coords);

    vec3 N   = normalize(fs_in.normal_view);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

	//return vec3(0,0,-1);
	//return fs_in.normalWorld;
    return normalize(TBN * tangentNormal);
}




void main()
{    		
	//vec3 normal = getNormalFromMap();
	float factor = 255/128.0f;
	
	vec3 N = (texture(material.normalMap, fs_in.tex_coords).xyz * factor) - 1.0;
	
	//normal = vec3(0,0,1);
	
	// transform normal from tangent to eye space
	N = normalize(fs_in.TBN_eye_directions * N);
	//normal = vec3(0,0,1);
	
	//normal = getNormalFromMap();
	
	
    // phase 1: directional lighting
	
	// calculate per-light radiance
	vec3 L =  normalize(fs_in.light_direction_eye);
	vec3 V = normalize(-fs_in.fragment_position_eye);
	
    vec3 result = pbrModel(N, V, L);

		
	//directional shadow calculation
	float shadow = shadowCalculation(L, N, fs_in.fragment_position_lightspace);
	
	//shadow *= shadow;
	//shadow *= shadow;
	//shadow *= shadow;
	//shadow *= shadow;
	
	vec3 albedoColor = texture(material.albedoMap, fs_in.tex_coords).rgb;
	
	if (shadow < 0.2) {
		shadow = 0.2;
	}

	result *= (shadow);
	
	//FragColor = vec4(normal, 1.0);
	
	
	
	/*vec3 incident = normalize(fs_in.fragment_position_eye);
    vec3 reflected = reflect(incident, normal);
	
	reflected = vec3(inverse(view) * vec4(reflected, 0.0));
	
	result= textureLod(prefilterMap, reflected, 0).rgb;*/
	
	//result = reflectanceWorldSpace();
	
	//result = reflectanceEyeSpace();
	
	//result = albedoColor;
	
	
	
	
	FragColor = vec4(result, 1.0);

	// hdr tone mapping  TODO move hdr tone mapping to a seperate post processing effect, as we need the hdr color value for other post processing effects (like bloom)
  
    // reinhard tone mapping
	//const float exposure = 1; // Todo: create a uniform from it
    //vec3 mapped = vec3(1.0) - exp(-result * exposure);
    //FragColor = vec4(mapped, 1.0);
}

vec3 pbrModel(vec3 N, vec3 V, vec3 L) {
	
	// material properties
    //vec3 albedo = pow(texture(material.albedoMap, fs_in.tex_coords).rgb, vec3(2.2));
	vec3 albedo = texture(material.albedoMap, fs_in.tex_coords).rgb;
	
	//albedo = vec3(1,0,0);
	
    float metallic = texture(material.metallicMap, fs_in.tex_coords).r;
	metallic = 0.0f;
    float roughness = texture(material.roughnessMap, fs_in.tex_coords).r;
	roughness = 0.5f;
    float ao = texture(material.aoMap, fs_in.tex_coords).r;
	
	ao = 1.0;
   
   
    vec3 R = reflect(-V, N);
	
	// R is in eye space, but we need it to be in world space
	R = vec3(inverse(view) * vec4(R, 0.0f));
	

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = pbrDirectLight(V, N, L, roughness, F0, metallic, albedo);
	
	vec3 ambient = pbrAmbientLight(V, N, roughness, F0, metallic, albedo, R, ao);
    
    vec3 color = Lo + ambient;
	
	return color;
}

vec3 pbrDirectLight(vec3 V, vec3 N, vec3 L, float roughness, vec3 F0, float metallic, vec3 albedo) {
	
	vec3 H = normalize(V + L);
	
	// directional lights have no distance and thus also no attenuation
	//float distance = length(lightPosition - fragPos);
	//float attenuation = 1.0 / (distance * distance);
	
	//243 159 24
	
	//vec3 radiance = vec3(243/ 255.0f, 159 / 255.0f, 24 / 255.0f) * 1.0f;//dirLight.color; /** attenuation*/
	vec3 radiance = vec3(1,1,1) * 1.0f;//dirLight.color; /** attenuation*/

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
	return ((kD * albedo / PI) + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again	
}

vec3 pbrAmbientLight(vec3 V, vec3 N, float roughness, vec3 F0, float metallic, vec3 albedo, vec3 R, float ao) {
	// ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
	
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb * 1.0f;
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	//brdf = vec2(1.0, 0.0);
	//brdf = vec2(1,1);
    vec3 ambientLightSpecular = prefilteredColor * (F * brdf.x + brdf.y);

    return (kD * diffuse + ambientLightSpecular) * ao;
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


float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragment_position_lightspace)
{		
		//vec3 shadowCoordinateWdivide  = (fragment_position_lightspace.xyz / fragment_position_lightspace.w) * 0.5 + vec3(0.5) ;
		//vec3 shadowCoordinateWdivide = fragment_position_lightspace.xyz * 0.5 + vec3(0.5) ;
		vec3 shadowCoordinateWdivide = fragment_position_lightspace.xyz / fragment_position_lightspace.w;
		
		float currentDepth = shadowCoordinateWdivide.z;
		float angle = dot(lightDir, normal);
		vec2 texelSize = vec2(1.0)/textureSize(material.shadowMap, 0);
		
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
		
		float depth = texture2D(material.shadowMap, shadowCoordinateWdivide.xy).r;
		
		float diff =  abs(currentDepth - depth);
	
		

		bias = 9.0f * max(texelSize.x,texelSize.y);
		//bias = 5.0f * 1.0f / 1920.0f;
		
		shadow = PCF(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);
		//shadow = texture2DCompare(material.shadowMap, shadowCoordinateWdivide.xy, currentDepth, bias);
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

float texture2DCompare(sampler2D depths, vec2 uv, float compare, float bias){
    float depth = texture2D(depths, uv).r;
		if (depth >= 1.0)
				return 0.0f;
    return step(depth + bias, compare);
}

float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare, float bias){
    vec2 texelSize = vec2(1.0)/size;
    vec2 f = fract(uv*size+0.5);
    vec2 centroidUV = floor(uv*size+0.5)/size;

    float lb = texture2DCompare(depths, centroidUV+texelSize*vec2(0.0, 0.0), compare, bias);
    float lt = texture2DCompare(depths, centroidUV+texelSize*vec2(0.0, 1.0), compare, bias);
    float rb = texture2DCompare(depths, centroidUV+texelSize*vec2(1.0, 0.0), compare, bias);
    float rt = texture2DCompare(depths, centroidUV+texelSize*vec2(1.0, 1.0), compare, bias);
    float a = mix(lb, lt, f.y);
    float b = mix(rb, rt, f.y);
    float c = mix(a, b, f.x);
    return c;
}

float PCF(sampler2D depths, vec2 size, vec2 uv, float compare, float bias){
    float result = 0.0;
		float xSamples = 2;
		float ySamples = 2;
		float sampleCount = (2*xSamples + 1) * (2*ySamples + 1);
    for(float x=-xSamples; x<=xSamples; x += 1){
        for(float y=-ySamples; y<=ySamples; y += 1){
            vec2 off = vec2(x,y)/size;
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