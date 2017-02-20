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

vec4 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec4 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec4 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace);
//vec4 shadowCalculation(vec4 fragPosLightSpace);

void main()
{    
    vec3 normal = normalize(fs_in.normal);
    vec3 viewDirection = normalize(viewPos - fs_in.fragPos);
    
    // phase 1: directional lighting
    vec4 result = calcDirLight(dirLight, normal, viewDirection);
    
    // phase 2: point lights
    //for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
    //    result += calcPointLight(pointLights[i], normal, fs_in.fragPos, viewDirection);
    //}
    
    // phase 3: spot lighting
    //result += calcSpotLight(spotLight, normal, fs_in.fragPos, viewDirection);
    
    // Reflection
    vec3 I = normalize(fs_in.reflectPosition - viewPos);
    vec3 R = reflect(I, normal);
    float reflect_intensity = texture(material.reflectionMap, fs_in.texCoords).r;
    vec4 reflect_color = vec4(0,0,0,1);
    if(reflect_intensity > 0.1) // Only sample reflections when above a certain treshold
        reflect_color = texture(skybox, R) * reflect_intensity;
           
    result += reflect_color;    
		result.w = 1.0f;
    result = clamp(result, 0.0, 1.0);
    
    color = result;
}

// Calculates the color when using a directional light source
vec4 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
		// we need the direction from the fragment to the light source, so we use the negative light direction!
    vec3 lightDir = normalize(-light.direction);
    vec4 diffuseColor = texture(material.diffuseMap, fs_in.texCoords);
    vec4 specularColor = texture(material.specularMap, fs_in.texCoords);
    // diffuse shading
    float diffuseAngle = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float shininess = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess * 2.0f);
    // combine results
    vec4 ambient = light.ambient * diffuseColor;
    vec4 diffuse = light.diffuse * diffuseAngle * diffuseColor;
    vec4 specular = light.specular * shininess * specularColor;
		
		//return shadowCalculation(fs_in.fragPosLightSpace);
		float shadow = shadowCalculation(lightDir, normal, fs_in.fragPosLightSpace);     
    //vec4 lighting = /*ambient +*/ (1.0 - shadow) * (diffuse + specular);
		vec4 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    return lighting;
		//return diffuse + specular;
		//return (shadow) * vec4(1,1,1,1);
		//return (1.0 - shadow) * (diffuse + specular);
		//return ambient;
		//return lighting;
		//return (ambient + diffuse + specular);
		//return diffuse;
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
    vec4 ambient = light.ambient * diffuseColor;
    vec4 diffuse = light.diffuse * diffuseAngle * diffuseColor;
    vec4 specular = light.specular * shininess * specularColor;
    return attenuation * (ambient + diffuse + specular);
}

// Calculates the color when using a spot light source
vec4 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec4 diffuseColor = texture(material.diffuseMap, fs_in.texCoords);
    vec4 specularColor = texture(material.specularMap, fs_in.texCoords);    
    // Ambient
    vec4 ambient = light.ambient * diffuseColor;
    
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
    ambient  *= attenuation; 
    diffuse  *= attenuation;
    specular *= attenuation;   
            
    return (ambient + diffuse + specular);  
}

float shadowCalculation(vec3 lightDir, vec3 normal, vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Check whether current frag pos is in shadow
		float bias = max(0.000001*(1.0 - dot(normal, lightDir)) ,0.000001);
		float shadow = 0.0;
		vec2 texelSize = 1.0 / textureSize(material.shadowMap, 0);
		for (int x = -1; x <= 1; ++x) {
			for (int y = -1; y <= 1; ++y) {
			  // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
				float closestDepth = texture(material.shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
			  shadow += currentDepth - bias > closestDepth  ? 1.0 : 0.0;
			}
		}
		shadow /= 9.0;
		
		//float closestDepth = texture(material.shadowMap, projCoords.xy).r;
	  //shadow += currentDepth - bias > closestDepth  ? 1.0 : 0.0;
		
		if (currentDepth > 1.0)
				shadow = 0.0;		
    return shadow;
} 