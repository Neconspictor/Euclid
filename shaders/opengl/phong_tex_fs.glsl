#version 330 core

struct Material {
    vec3 ambient;
    sampler2D diffuseMap;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
    vec3 diffuseColor = vec3(texture(material.diffuseMap, texCoordsFS));
    
    
    vec3 ambient = diffuseColor * light.ambient;
    
    vec3 normal = normalize(normalVS);
    vec3 lightDirection = normalize(light.position - fragmentPosition);
    
    float diffuseAngle = max(dot(normal, lightDirection), 0.0f);
    vec3 diffuse = (diffuseAngle * diffuseColor) * light.diffuse;
        
    // camera position is at (0,0,0) in view space, view direction is cameraPosition - fragmentPosition
    // results in -fragmentPosition
    vec3 viewDirection = normalize(viewPos - fragmentPosition);
    
    // lightDirection points to the light source, but reflect expects a vector pointing away from the light source
    // thus -lightDirection is needed
    vec3 reflectDirection = reflect(-lightDirection, normal);
    
    float shininess = pow(max(dot(viewDirection, reflectDirection), 0.0f), material.shininess);
    
    vec3 specular =  (shininess * material.specular) * light.specular;
    
    vec3 result = clamp(ambient + diffuse + specular, 0.0, 1.0);
    color = vec4(result, 1.0f);
}