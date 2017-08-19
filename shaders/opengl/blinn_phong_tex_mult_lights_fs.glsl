#version 330 core
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

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    
    // cosine of the angle of the spot light radius
    float cutOff;
    
    // A second angle for defining smooth edges
    float outerCutOff;
    
    float constant;
    float linear;
    float quadratic;
    
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

in VS_OUT {
	vec3 fragPos;
	vec2 texCoords;
	vec4 fragPosLightSpace; // needed for shadow calculation
	vec3 TangentLightPos;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
	mat3 TBN;
} fs_in;


out vec4 color;

uniform Material material;
uniform samplerCube skybox;
uniform vec3 viewPos;

uniform DirLight dirLight;
uniform PointLight pointLights [NR_POINT_LIGHTS];
uniform SpotLight spotLight;

//for point light shadows
uniform samplerCube cubeDepthMap;
uniform float range;

vec4 calcAmbient(DirLight light, vec3 normal, vec3 viewDir);
vec4 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec4 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec4 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace);
float shadowCalculationVariance(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace);
float pointShadowCalculation(PointLight light);
float texture2DCompare(sampler2D depths, vec2 uv, float compare, float bias);
float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float PCF(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float chebyshevUpperBound( float distance, vec2 uv);

void main()
{    
    //vec3 normal = normalize(fs_in.normal);
	
	vec3 normal = texture(material.normalMap, fs_in.texCoords).rgb; 
	vec3 normalLighting = normal;
	normalLighting = normalize(fs_in.TBN * normalLighting); //normalize(normal * 2.0 - 1.0);
	vec3 normalShadows = normalize(fs_in.TBN * normal);
	//normal = normalize(normal * 2.0 - 1.0); 
	//normal = vec3(0,0,1);
	//normal =  fs_in.TBN * normal;
	
	//vec3 defaultNormal = normalize(fs_in.TBN * fs_in.normal);
	//defaultNormal = fs_in.normal;
	
    vec3 viewDirection = normalize(viewPos - fs_in.fragPos);
	
    // phase 1: directional lighting
    vec4 dirLightColor = calcDirLight(dirLight, normalLighting, viewDirection);
	vec4 ambientColor = calcAmbient(dirLight, normalLighting, viewDirection);
	vec4 result = dirLightColor;
	//result.x = max(dirLightColor.x, ambientColor.x);
	//result.y = max(dirLightColor.y, ambientColor.y);
	//result.z = max(dirLightColor.z, ambientColor.z);
		
    // phase 2: point lights
    for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        //result += calcPointLight(pointLights[i], normal, fs_in.fragPos, viewDirection);
    }
    
    // phase 3: spot lighting
    //result += calcSpotLight(spotLight, normal, fs_in.fragPos, viewDirection);
    
    // phase 4: Reflection
    vec3 I = normalize(fs_in.fragPos - viewPos);
    vec3 R = reflect(I, normalLighting);
    float reflect_intensity = texture(material.reflectionMap, fs_in.texCoords).r;
    vec4 reflect_color = vec4(0,0,0,1);
    if(reflect_intensity > 0.1) // Only sample reflections when above a certain treshold
        reflect_color = texture(skybox, R) * reflect_intensity;
           
    //result += reflect_color;    
		
		
		//directional shadow calculation
		float shadow = shadowCalculation(normalize(dirLight.direction), normalShadows, fs_in.fragPosLightSpace);
		//float shadow = shadowCalculationVariance(normalize(dirLight.direction), normal, fs_in.fragPosLightSpace);
		
		//spot light shadows
		for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
		  //shadow *= pointShadowCalculation(pointLights[i]);
		}
		
		
		//result *= 1 - shadow;
		
		result += ambientColor;
		
		//shadow = min(0.9, shadow);
		
		
		
		if (shadow > 0.0f) {
		  //result = vec4(shadow, shadow, shadow, 1);
		}
		
		
		result.w = 1.0f;
    //result = clamp(result, 0.0, 1.0);

    
    color = result;
}

// Calculates the color when using a directional light source
vec4 calcAmbient(DirLight light, vec3 normal, vec3 viewDir) {
		// we need the direction from the fragment to the light source, so we use the negative light direction!
    vec3 lightDir = normalize(-light.direction);
    vec4 diffuseColor = vec4(texture(material.diffuseMap, fs_in.texCoords).rgb, 1.0);
    
	// combine results
    vec4 diffuse = light.diffuse * diffuseColor;	
	return 0.005 * diffuse;
}

// Calculates the color when using a directional light source
vec4 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
		// we need the direction from the fragment to the light source, so we use the negative light direction!
    vec3 lightDir = normalize(fs_in.TBN * (-light.direction));
    vec4 diffuseColor = vec4(texture(material.diffuseMap, fs_in.texCoords).rgb, 1.0);
    vec4 specularColor = vec4(texture(material.specularMap, fs_in.texCoords).rgb, 1.0);
    specularColor = vec4(1.0, 1.0, 1.0, 1);
	//specularColor = 2 * diffuseColor;
	
	/*// diffuse shading
    float diffuseAngle = max(dot(normal, lightDir), 0.0);
    
	// specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
	float shininess = 0.3 * pow(max(dot(normal, halfwayDir), 0.0), 32);
    
	// combine results
    vec4 diffuse = light.diffuse * diffuseAngle * diffuseColor;
	
    vec4 specular = vec4(vec3(light.specular * shininess *  specularColor), 1.0);		
	return diffuse + specular;*/
	
	// diffuse
    //lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diffuseAngle = max(dot(lightDir, normal), 0.0);
    vec4 diffuse = diffuseAngle * diffuseColor;
    // specular
    viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	//viewDir = normalize(fs_in.TBN * viewDir);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float shininess = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec4 specular = vec4(0.2, 0.2, 0.2, 1.0) * shininess;
	return diffuse + specular;
}

// Calculates the color when using a point light source
vec4 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fs_in.fragPos);
    vec4 diffuseColor = texture(material.diffuseMap, fs_in.texCoords);
    vec4 specularColor = texture(material.specularMap, fs_in.texCoords);
    // diffuse shading
    float diffuseAngle = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float shininess = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess * 2.0f);
    
    // attenuation
    float distance = length(light.position - fs_in.fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + 
                        light.quadratic * distance * distance);
      
    // combine results
    vec4 diffuse = light.diffuse * diffuseAngle * diffuseColor;
    vec4 specular = light.specular * shininess * specularColor;
    return attenuation * (diffuse + specular);
}

// Calculates the color when using a spot light source
vec4 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec4 diffuseColor = texture(material.diffuseMap, fs_in.texCoords);
    vec4 specularColor = texture(material.specularMap, fs_in.texCoords);    
    
    // Diffuse 
    vec3 norm = normalize(normal);        
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = light.diffuse * diff * diffuseColor;  
    
    // Specular
    //vec3 reflectDir = reflect(-lightDir, norm); 
    vec3 halfwayDir = normalize(lightDir + viewDir);      
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess * 2.0f);
    vec4 specular = light.specular * spec * specularColor;
    
    // Spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    
    // Attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    diffuse  *= attenuation;
    specular *= attenuation;   
            
    return (diffuse + specular);  
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


float pointShadowCalculation(PointLight light)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fs_in.fragPos.xyz - light.position;
    float closestDepth = texture(cubeDepthMap, fragToLight).r;
		float closestDepthRanged = closestDepth * range;
    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // Now test for shadows
    float bias = 0.05;
		currentDepth -= bias;
    float shadow = currentDepth > closestDepthRanged ? 1.0 : 0.0;

    return shadow;
		
		
	/*vec3 fragToLight = fs_in.fragPos.xyz - light.position;
	float currentDepth = length(fragToLight);
	
	float shadow = 0.0;
	float bias = 0.3; 
	float samples = 4.0;
	float offset = 0.01;
	for(float x = -offset; x < offset; x += offset / (samples * 0.5))
	{
		for(float y = -offset; y < offset; y += offset / (samples * 0.5))
		{
				for(float z = -offset; z < offset; z += offset / (samples * 0.5))
				{
						float closestDepth = texture(cubeDepthMap, fragToLight + vec3(x, y, z)).r; 
						closestDepth *= range;   // Undo mapping [0;1]
						if(currentDepth - bias > closestDepth)
								shadow += 1.0;
				}
		}
	}
	shadow /= (samples * samples * samples);
	return shadow;*/
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