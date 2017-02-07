#version 330 core

struct Material {
    sampler2D diffuseMap;
    sampler2D emissionMap;
    sampler2D reflectionMap;
    sampler2D specularMap;
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

#define NR_POINT_LIGHTS 4

in vec3 fragmentPosition;
in vec3 reflectPosition;
in vec3 normalVS;
in vec2 texCoordsFS;

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

void main()
{    
    vec3 normal = normalize(normalVS);
    vec3 viewDirection = normalize(viewPos - fragmentPosition);
    
    // phase 1: directional lighting
    vec4 result = calcDirLight(dirLight, normal, viewDirection);
    
    // phase 2: point lights
    for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        result += calcPointLight(pointLights[i], normal, fragmentPosition, viewDirection);
    }
    
    // phase 3: spot lighting
    //result += calcSpotLight(spotLight, normal, fragmentPosition, viewDirection);
    
    // Reflection
    vec3 I = normalize(reflectPosition - viewPos);
    vec3 R = reflect(I, normal);
    float reflect_intensity = texture(material.reflectionMap, texCoordsFS).r;
    vec4 reflect_color = vec4(0,0,0,1);
    if(reflect_intensity > 0.1) // Only sample reflections when above a certain treshold
        reflect_color = texture(skybox, R) * reflect_intensity;
           
    result += reflect_color;    
    
    result = clamp(result, 0.0, 1.0);
    
    color = result;
}

// Calculates the color when using a directional light source
vec4 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    vec4 diffuseColor = texture(material.diffuseMap, texCoordsFS);
    vec4 specularColor = texture(material.specularMap, texCoordsFS);
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
    return (ambient + diffuse + specular);
}

// Calculates the color when using a point light source
vec4 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragmentPosition);
    vec4 diffuseColor = texture(material.diffuseMap, texCoordsFS);
    vec4 specularColor = texture(material.specularMap, texCoordsFS);
    // diffuse shading
    float diffuseAngle = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float shininess = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess * 2.0f);
    
    // attenuation
    float distance = length(light.position - fragmentPosition);
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
    vec4 diffuseColor = texture(material.diffuseMap, texCoordsFS);
    vec4 specularColor = texture(material.specularMap, texCoordsFS);    
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