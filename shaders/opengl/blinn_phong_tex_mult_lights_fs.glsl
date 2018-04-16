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
	vec3 fragPos;
	vec2 texCoords;
	vec4 fragPosLightSpace; // needed for shadow calculation
	vec3 TangentFragPos;
	mat3 TBN;
	vec3 T;
	vec3 B;
	vec3 N;
	vec3 tangentLightDir;
	vec3 tangentViewDir;
	vec3 normal;
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

vec3 phongModel(vec3 normal);
vec4 calcDirLight(vec3 normal, vec3 viewDir);
float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace);
float shadowCalculationVariance(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace);
float texture2DCompare(sampler2D depths, vec2 uv, float compare, float bias);
float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float PCF(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float chebyshevUpperBound( float distance, vec2 uv);

void main()
{    	
	vec3 normal = texture(material.normalMap, fs_in.texCoords).rgb;
	//normal = vec3(128 / 255.0, 128 / 255.0, 255 / 255.0);
	normal = normalize(2.0*normal - 1.0);
	//normal = fs_in.normal;

	vec3 normalLighting = normalize(normal);
	vec3 normalShadows = normalize(normal);
	
    // phase 1: directional lighting
	//normalLighting = normalize(vec3(0,0,1));
    vec3 result = phongModel(normalLighting);
		
	//directional shadow calculation
	float shadow = shadowCalculation(normalize(fs_in.tangentLightDir), normalLighting, fs_in.fragPosLightSpace);
	//result *= (shadow);
	
	FragColor = vec4(result, 1.0);

	// hdr tone mapping  TODO move hdr tone mapping to a seperate post processing effect, as we need the hdr color value for other post processing effects (like bloom)
  
    // reinhard tone mapping
	//const float exposure = 1; // Todo: create a uniform from it
    //vec3 mapped = vec3(1.0) - exp(-result * exposure);
    //FragColor = vec4(mapped, 1.0);
}

vec3 phongModel(vec3 normal) {
	vec3 diffuseColor = texture(material.diffuseMap, fs_in.texCoords).rgb;
	//normal = vec3(0,0,1);
    vec3 specularColor = texture(material.specularMap, fs_in.texCoords).rgb;
	
	
	mat3 model3D = mat3(model);
	
	vec3 N = normalize((model3D * fs_in.N).xyz); // original normalMatrix
	vec3 T = normalize(model3D * fs_in.T);	// original normalMatrix
	
	float dotTN = dot(N, T);
	
	if (dotTN < 0.0) {
		//T = -1.0 * T;
	};
	
	T = normalize(T - (dot(N, T) * N));
	
	vec3 B = normalize(cross(N, T));

	
	
	//mat3 TBN = transpose(mat3(T, B, N));
	mat3 TBN = mat3(T, B, N);
	//mat3 TBN = fs_in.TBN;
	
	vec3 lightDir =  normalize(TBN * fs_in.tangentLightDir);
   // vec3 r = reflect(normal, lightDir);
    vec3 ambient = 0.1 * diffuseColor;
    float sDotN = max(dot(normal, lightDir), 0.0 );
    vec3 diffuse = diffuseColor * sDotN;
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 ) {
		vec3 viewDir = normalize(TBN * fs_in.tangentViewDir);
		vec3 halfwayDir = normalize(lightDir + viewDir); 
		float shininess = pow( max( dot(normal, halfwayDir), 0.0 ), 8.0 );
		//float shininess = pow( max( dot(r, viewDir), 0.0 ), 16.0 );
        spec = vec3(0.2) * shininess;	
	}
	
    return /*ambient +*/ diffuse + spec;
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
		
		float currentDepth = shadowCoordinateWdivide.z;
		float angle = dot(lightDir, normal);
		angle = tan(acos(angle));
		float bias = max(0.000005 * angle ,0.0000005);		
		bias = 0.003;
		//bias = 0.8;
	
		//float shadow = PCF(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);
		
		vec4 test = vec4(shadowCoordinateWdivide.xy, shadowCoordinateWdivide.z-bias, 1);
		
		//float shadow = textureProj(material.shadowMap, test, 0);
		float shadow = texture2DCompare(material.shadowMap, shadowCoordinateWdivide.xy, currentDepth, bias);
		//float shadow = PCF(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);
		//shadow = texture2DShadowLerp(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);
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
		float sampleCount = (4*xSamples + 1) * (4*ySamples + 1);
    for(float x=-xSamples; x<=xSamples; x += 0.5){
        for(float y=-ySamples; y<=ySamples; y += 0.5){
            vec2 off = vec2(x,y)/size;
            result += texture2DShadowLerp(depths, size, uv+off, compare, bias);
        }
    }
    return result / (sampleCount);
}