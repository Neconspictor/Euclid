#version 400
#define NR_POINT_LIGHTS 4

struct Material {
    sampler2D diffuseMap;
    sampler2D emissionMap;
    sampler2D normalMap;
	sampler2D reflectionMap;
    sampler2D specularMap;
	sampler2D shadowMap;
	sampler2D vsMap;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};


in VS_OUT {
	vec3 fragment_position_world;
	vec2 tex_coords;
	vec4 fragment_position_lightspace; // needed for shadow calculation
	vec3 view_direction_tangent; // the view direction in tangent space
	vec3 light_direction_tangent; // the light direction in tangent space
} fs_in;


out vec4 FragColor;

uniform DirLight dirLight;

uniform Material material;
uniform samplerCube skybox;
uniform vec3 viewPos;

uniform mat4 model;
uniform mat4 modelView;

//for point light shadows
uniform samplerCube cubeDepthMap;
uniform float range;


uniform mat3 normalMatrix;



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









vec3 phongModel(vec3 normal);
vec4 calcDirLight(vec3 normal, vec3 viewDir);
float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace);
float shadowCalculationVariance(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace);
float texture2DCompare(sampler2D depths, vec2 uv, float compare, float bias);
float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float PCF(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float chebyshevUpperBound( float distance, vec2 uv);

float PCSS (sampler2D shadowMap, vec3 coords, float bias);
float PCF_Filter(sampler2D shadowMap, vec2 uv, float zReceiver, float filterRadiusUV, float bias);

void main()
{    	
	vec3 normal = texture(material.normalMap, fs_in.tex_coords).rgb;
	//normal = vec3(128 / 255.0, 128 / 255.0, 255 / 255.0);
	normal = normalize(2.0*normal - 1.0);

	//normal = vec3(0,0,1);
	vec3 normalLighting = normalize(normal);
	
	
    // phase 1: directional lighting
	//normalLighting = normalize(vec3(0,0,1));
    vec3 result = phongModel(normalLighting);
		
	//directional shadow calculation
	float shadow = shadowCalculation(normalize(fs_in.light_direction_tangent), normalLighting, fs_in.fragment_position_lightspace);
	
	vec3 diffuseColor = texture(material.diffuseMap, fs_in.tex_coords).rgb;
	
	result *= (shadow);
	
	//vec3 ambient = 0.1 * diffuseColor;
	
	//if (result.r < ambient.r) {
		//result = ambient;
	//};
	
	FragColor = vec4(result, 1.0);

	// hdr tone mapping  TODO move hdr tone mapping to a seperate post processing effect, as we need the hdr color value for other post processing effects (like bloom)
  
    // reinhard tone mapping
	//const float exposure = 1; // Todo: create a uniform from it
    //vec3 mapped = vec3(1.0) - exp(-result * exposure);
    //FragColor = vec4(mapped, 1.0);
}

vec3 phongModel(vec3 normal) {
	vec3 diffuseColor = texture(material.diffuseMap, fs_in.tex_coords).rgb;
	//normal = vec3(0,0,1);
    vec3 specularColor = texture(material.specularMap, fs_in.tex_coords).rgb;
	
	
	vec3 lightDir =  normalize(fs_in.light_direction_tangent);
   // vec3 r = reflect(normal, lightDir);
    vec3 ambient = 0.1 * diffuseColor;
    float sDotN = max(dot(lightDir, normal), 0.0 );
    vec3 diffuse = diffuseColor * sDotN;
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 ) {
		vec3 viewDir = normalize(fs_in.view_direction_tangent);
		vec3 halfwayDir = normalize(lightDir + viewDir); 
		float shininess = pow( max( dot(normal, halfwayDir), 0.0 ), 8.0 );
		//float shininess = pow( max( dot(r, viewDir), 0.0 ), 16.0 );
        spec = vec3(0.2) * shininess;	
	}
	
    return ambient + diffuse + spec;
}


float chebyshevUpperBound( float distance, vec2 uv)
	{
		// We retrive the two moments previously stored (depth and depth*depth)
		vec2 moments = texture2D(material.vsMap, uv).rg;
		
		// Surface is fully lit. as the current fragment is before the light occluder
		if (distance <= moments.x)
			return 1.0 ;
	
		// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
		// How likely this pixel is to be lit (p_max)
		//moments.y = moments.x*moments.x;
		float variance = moments.y - (moments.x*moments.x);
		//variance = max(variance,0.000000003);
	
		float d = distance - moments.x;
		float p_max = variance / (variance + d*d);
	
		return p_max;
	}

float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace)
{		
		vec3 shadowCoordinateWdivide  = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + vec3(0.5) ;
		//vec3 shadowCoordinateWdivide = fragPosLightSpace.xyz * 0.5 + vec3(0.5) ;
		//shadowCoordinateWdivide = fragPosLightSpace.xyz;
		
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
	
		

		bias = 4.0f * texelSize.x;
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

float shadowCalculationVariance(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace) {
		if (fragPosLightSpace.z >= 1.0)
				return 0.0;
	
		return 1 - chebyshevUpperBound(fragPosLightSpace.z, fragPosLightSpace.xy);
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