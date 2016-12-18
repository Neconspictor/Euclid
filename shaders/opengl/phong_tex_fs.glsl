#version 330 core

struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D emissionMap;
    float shininess;
};

struct Light {
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

in vec3 fragmentPosition;
in vec3 normalVS;
in vec2 texCoordsFS;

out vec4 color;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    // sample the diffuse color for the current fragment
    vec4 diffuseColor = texture(material.diffuseMap, texCoordsFS);
    
    // sample the specular color for the current fragment
    vec4 specularColor = texture(material.specularMap, texCoordsFS);
    
    // sample the emmission color for the current fragment
    vec4 emissionColor = texture(material.emissionMap, texCoordsFS);
    
    
    vec4 ambient = diffuseColor * light.ambient;
    
    vec3 normal = normalize(normalVS);
    vec3 lightDirection = normalize(light.position - fragmentPosition);
    
    float diffuseAngle = max(dot(normal, lightDirection), 0.0f);
    vec4 diffuse = (diffuseAngle * diffuseColor) * light.diffuse;
        
    // camera position is at (0,0,0) in view space, view direction is cameraPosition - fragmentPosition
    // results in -fragmentPosition
    vec3 viewDirection = normalize(viewPos - fragmentPosition);
    
    // lightDirection points to the light source, but reflect expects a vector pointing away from the light source
    // thus -lightDirection is needed
    vec3 reflectDirection = reflect(-lightDirection, normal);
    
    float shininess = pow(max(dot(viewDirection, reflectDirection), 0.0f), material.shininess);
    
    vec4 specular =  (shininess * specularColor) * light.specular;
    
    color = clamp(ambient + diffuse + specular + emissionColor, 0.0, 1.0);
}