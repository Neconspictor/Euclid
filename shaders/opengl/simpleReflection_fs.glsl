#version 330 core
in vec3 Normal;
in vec3 Position;
out vec4 color;

uniform vec3 cameraPos;
uniform samplerCube reflectionTexture;

void main()
{  
    // reflection
    //vec3 I = normalize(Position - cameraPos);
    //vec3 R = reflect(I, normalize(Normal));
    
    // refraction
    float ratio = 1.00 / 1.309;
    vec3 I = normalize(Position - cameraPos);
    vec3 R = refract(I, normalize(Normal), ratio);
    
    color = texture(reflectionTexture, R);
}