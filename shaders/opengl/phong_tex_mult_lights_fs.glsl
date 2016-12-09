#version 330 core

struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D emissionMap;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4

in vec3 fragmentPosition;
in vec3 normalVS;
in vec2 texCoordsFS;

out vec4 color;

uniform Material material;
uniform vec3 viewPos;

uniform DirLight dirLight;
uniform PointLight pointLights [NR_POINT_LIGHTS];
uniform SpotLight spotLight;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    vec3 normal = normalize(normalVS);
    vec3 viewDirection = normalize(viewPos - fragmentPosition);
    
    // phase 1: directional lighting
    vec3 result = calcDirLight(dirLight, normal, viewDirection);
    
    // phase 2: point lights
    for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        result += calcPointLight(pointLights[i], normal, fragmentPosition, viewDirection);
    }
    
    // phase 3: spot lighting
    //result += calcSpotLight(spotLight, normal, fragmentPosition, viewDirection);
    
    result = clamp(result, 0.0, 1.0);
    
    color = vec4(result, 1.0f);
}

// Calculates the color when using a directional light source
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    vec3 diffuseColor = vec3(texture(material.diffuseMap, texCoordsFS));
    vec3 specularColor = vec3(texture(material.specularMap, texCoordsFS));
    // diffuse shading
    float diffuseAngle = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float shininess = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diffuseAngle * diffuseColor;
    vec3 specular = light.specular * shininess * specularColor;
    return (ambient + diffuse + specular);
}

// Calculates the color when using a point light source
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragmentPosition);
    vec3 diffuseColor = vec3(texture(material.diffuseMap, texCoordsFS));
    vec3 specularColor = vec3(texture(material.specularMap, texCoordsFS));
    // diffuse shading
    float diffuseAngle = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float shininess = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // attenuation
    float distance = length(light.position - fragmentPosition);
    float attenuation = 1.0f / (light.constant + light.linear * distance + 
                        light.quadratic * distance * distance);
      
    // combine results
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diffuseAngle * diffuseColor;
    vec3 specular = light.specular * shininess * specularColor;
    return attenuation * (ambient + diffuse + specular);
}

// Calculates the color when using a spot light source
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 diffuseColor = vec3(texture(material.diffuseMap, texCoordsFS));
    vec3 specularColor = vec3(texture(material.specularMap, texCoordsFS));    
    // Ambient
    vec3 ambient = light.ambient * diffuseColor;
    
    // Diffuse 
    vec3 norm = normalize(normal);        
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseColor;  
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularColor;
    
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