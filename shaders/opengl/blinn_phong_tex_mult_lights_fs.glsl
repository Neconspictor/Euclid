#version 330 core
#define NR_POINT_LIGHTS 4

struct Material {
    sampler2D diffuseMap;
    sampler2D emissionMap;
    sampler2D reflectionMap;
    sampler2D specularMap;
		sampler2D shadowMap;
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
	vec3 normal;
	vec2 texCoords;
  vec3 reflectPosition;
	vec4 fragPosLightSpace;
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

vec4 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec4 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec4 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace);
float pointShadowCalculation(PointLight light);
float texture2DCompare(sampler2D depths, vec2 uv, float compare, float bias);
float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);
float PCF(sampler2D depths, vec2 size, vec2 uv, float compare, float bias);

void main()
{    
    vec3 normal = normalize(fs_in.normal);
    vec3 viewDirection = normalize(viewPos - fs_in.fragPos);
    
    // phase 1: directional lighting
    vec4 result = calcDirLight(dirLight, normal, viewDirection);
		
    // phase 2: point lights
    for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        //result += calcPointLight(pointLights[i], normal, fs_in.fragPos, viewDirection);
    }
    
    // phase 3: spot lighting
    //result += calcSpotLight(spotLight, normal, fs_in.fragPos, viewDirection);
    
    // phase 4: Reflection
    vec3 I = normalize(fs_in.reflectPosition - viewPos);
    vec3 R = reflect(I, normal);
    float reflect_intensity = texture(material.reflectionMap, fs_in.texCoords).r;
    vec4 reflect_color = vec4(0,0,0,1);
    if(reflect_intensity > 0.1) // Only sample reflections when above a certain treshold
        reflect_color = texture(skybox, R) * reflect_intensity;
           
    result += reflect_color;    
		
		
		//directional shadow calculation
		float shadow = shadowCalculation(normalize(dirLight.direction), normal, fs_in.fragPosLightSpace);

		//spot light shadows
		for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
		  //shadow *= pointShadowCalculation(pointLights[i]);
		}
		
		
		
		result *= 1 - shadow;
		
		// phase 5: ambient lighting
		vec4 diffuseColor = texture(material.diffuseMap, fs_in.texCoords);
		result += dirLight.ambient * diffuseColor;
		
		if (shadow > 0.0f) {
		  //result = vec4(shadow, shadow, shadow, 1);
		}
		
		
		result.w = 1.0f;
    //result = clamp(result, 0.0, 1.0);
    
    color = result;
}

// Calculates the color when using a directional light source
vec4 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
		// we need the direction from the fragment to the light source, so we use the negative light direction!
    vec3 lightDir = normalize(light.direction);
    vec4 diffuseColor = texture(material.diffuseMap, fs_in.texCoords);
    vec4 specularColor = texture(material.specularMap, fs_in.texCoords);
    // diffuse shading
    float diffuseAngle = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float shininess = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess * 2.0f);
    // combine results
    vec4 diffuse = light.diffuse * diffuseAngle * diffuseColor;
    vec4 specular = light.specular * shininess * specularColor;
		
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

float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace)
{
    /*// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Check whether current frag pos is in shadow
		float bias = max(0.001*(1.0 - dot(normal, lightDir)) ,0.001);
		
		float shadow = 0.0;		
		float closestDepth = texture(material.shadowMap, projCoords.xy).r;
	  shadow += currentDepth - bias > closestDepth  ? 1.0 : 0.0;
		
		if (currentDepth > 1.0)
				shadow = 0.0;
    return shadow;*/
		
		
		vec3 shadowCoordinateWdivide  = fragPosLightSpace.xyz;// / fragPosLightSpace.w ;
		//shadowCoordinateWdivide = shadowCoordinateWdivide * 0.5 + 0.5;
		
		// Used to lower moiré pattern and self-shadowing
		//shadowCoordinateWdivide.z += 0.0005;
		
		float currentDepth = shadowCoordinateWdivide.z;
		float bias = max(0.0005*(1.0 - dot(normal, -lightDir)) ,0.0005);		
	
		float shadow = 0.0;
		
	  vec2 texelSize = 1.0 / textureSize(material.shadowMap, 0);
	  /*for(float x = -1.5; x <= 1.5; ++x)
	  {
      for(float y = -1.5; y <= 1.5; ++y)
      {
        float closestDepth = texture(material.shadowMap, shadowCoordinateWdivide.xy + vec2(x, y) * texelSize).r; 
        shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;        
				//shadow += texture2DShadowLerp(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);
      }    
	  }
	  shadow /= 16.0;*/
		shadow = PCF(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);
		//shadow = texture2DCompare(material.shadowMap, shadowCoordinateWdivide.xy, currentDepth, bias);
		//shadow = texture2DShadowLerp(material.shadowMap, textureSize(material.shadowMap, 0), shadowCoordinateWdivide.xy, currentDepth, bias);	
	 	//if (fragPosLightSpace.w > 0.0)
		float closestDepth = texture2D(material.shadowMap, shadowCoordinateWdivide.xy).r;
	 	//shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0 ;		
    return shadow;
		
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