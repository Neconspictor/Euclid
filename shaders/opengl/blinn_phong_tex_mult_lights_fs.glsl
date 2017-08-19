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
	vec3 tangentLightDir;
	vec3 tangentViewDir;
	vec3 viewLightDir;
} fs_in;


out vec4 FragColor;

uniform DirLight dirLight;

uniform Material material;
uniform samplerCube skybox;
uniform vec3 viewPos;

uniform mat4 model;

//for point light shadows
uniform samplerCube cubeDepthMap;
uniform float range;

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
    //vec3 normal = normalize(fs_in.normal);
	
	vec3 normal = texture(material.normalMap, fs_in.texCoords).rgb; 
	vec3 normalLighting = vec3(mat3(model) * normal);
	normalLighting = normalize(fs_in.TBN * normalLighting); // normalize(fs_in.TBN * normalLighting); //
	vec3 normalShadows = normal;
	
    // phase 1: directional lighting
    vec3 result = phongModel(normalLighting);
		
	//directional shadow calculation
	float shadow = shadowCalculation(normalize(fs_in.viewLightDir), normalShadows, fs_in.fragPosLightSpace);
	//result *= (1 - shadow);
	
	FragColor = vec4(result, 1.0);
}

vec3 phongModel(vec3 normal) {
	vec3 diffuseColor = texture(material.diffuseMap, fs_in.texCoords).rgb;
    vec3 specularColor = texture(material.specularMap, fs_in.texCoords).rgb;
	
	vec3 lightDir = fs_in.tangentLightDir;
    vec3 r = reflect( -lightDir, normal );
    vec3 ambient = 0.0 * diffuseColor;
    float sDotN = max( dot(lightDir, normal), 0.0 );
    vec3 diffuse = diffuseColor * sDotN;
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 ) {
		vec3 viewDir = normalize(fs_in.TBN * (viewPos - fs_in.fragPos));
		float shininess = pow( max( dot(r, viewDir), 0.0 ), 32.0 );
        spec = vec3(0.3) * shininess;	
	}
	
    return ambient + diffuse + spec;
}

// Calculates the color when using a directional light source
vec4 calcDirLight(vec3 normal, vec3 viewDir) {
		// we need the direction from the fragment to the light source, so we use the negative light direction!
    vec3 lightDir = fs_in.tangentLightDir;
    vec4 diffuseColor = vec4(texture(material.diffuseMap, fs_in.texCoords).rgb, 1.0);
    vec4 specularColor = vec4(texture(material.specularMap, fs_in.texCoords).rgb, 1.0);
    specularColor = vec4(1.0, 1.0, 1.0, 1);
	
	
	// diffuse
    //lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diffuseAngle = max(dot(-lightDir, normal), 0.0);
    vec4 diffuse = diffuseAngle * diffuseColor;
    // specular
    //viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	viewDir = normalize(fs_in.TBN * viewDir);
    vec3 reflectDir = reflect(lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float shininess = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec4 specular = specularColor * shininess;
	return diffuse + specular;
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
		vec3 shadowCoordinateWdivide  = fragPosLightSpace.xyz;
		
		float currentDepth = shadowCoordinateWdivide.z;
		float angle = dot(normal, -lightDir);
		angle = tan(acos(angle));
		float bias = max(0.000005 * angle ,0.0000005);		
		bias = 0.0;
		//bias = 0.8;
	
		float shadow = PCF(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);
		//shadow = texture2DCompare(material.shadowMap, shadowCoordinateWdivide.xy, currentDepth, bias);
		//shadow = texture2DShadowLerp(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);	
    return shadow;	
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
		float xSamples = 0.5;
		float ySamples = 0.5;
		float sampleCount = (2*xSamples+1) * (2*ySamples+1);
    for(float x=-xSamples; x<=xSamples; x++){
        for(float y=-ySamples; y<=ySamples; y++){
            vec2 off = vec2(x,y)/size;
            result += texture2DShadowLerp(depths, size, uv+off, compare, bias);
        }
    }
    return result / sampleCount;
}