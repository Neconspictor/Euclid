#version 330 core
in vec3 normalVS;
in vec3 fragmentPosition;

out vec4 color;
  
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;

void main()
{

    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;
    
    vec3 normal = normalize(normalVS);
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    
    float diffuseAngle = max(dot(normal, lightDirection), 0.0f);
    vec3 diffuse = diffuseAngle * lightColor;
    
    vec3 result = (ambient + diffuse) * objectColor;
    color = vec4(result, 1.0f);
}