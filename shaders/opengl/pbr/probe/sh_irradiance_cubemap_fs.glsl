#version 460 core

out vec4 FragColor;
in vec3 localPos;

layout (binding = 0) uniform sampler2D coefficients;

const float PI = 3.14159265359;


float getCosineLobeFactor(int index) {
    if(index==0) {
        return PI;
    }
    else if(index==1){
        return 2.0/3.0 * PI;
    }
    else if(index==2){
        return 2.0/3.0 * PI;
    }
    else if(index==3){
        return 2.0/3.0 * PI;
    }
    else if(index==4){
        return PI/ 4.0;
    }
    else if(index==5){
        return PI/ 4.0;
    }
    else if(index==6){
        return PI/ 4.0;
    }
    else if(index==7){
        return PI/ 4.0;
    }
    else {
        return PI/ 4.0;
    }
}

float harmonics(int index, in vec3 direction) {
    
    if(index==0) {
        return 0.282095;
    }
    else if(index==1){
        return 0.488603 * direction.y;
    }
    else if(index==2){
        return 0.488603 * direction.z;
    }
    else if(index==3){
        return 0.488603 * direction.x;
    }
    else if(index==4){
        return 1.092548 * direction.x * direction.y;
    }
    else if(index==5){
        return 1.092548 * direction.y * direction.z;
    }
    else if(index==6){
        return 0.315392 * (3.0 * direction.z * direction.z - 1.0);
    }
    else if(index==7){
        return 1.092548 * direction.x * direction.z;
    }
    else {
        return 0.546274 * (direction.x * direction.x - direction.y * direction.y);
    }
}

void main()
{		
    // the sample direction equals the hemisphere's orientation 
    /*vec3 normal = normalize(localPos);
  
    vec3 irradiance = vec3(0.0);  

	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, normal);
	up         = cross(normal, right);

	float sampleDelta = 0.025;
	float nrSamples = 0.0; 
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));
  
    FragColor = vec4(irradiance, 1.0);*/
    
    
    vec3 normal = normalize(localPos);
    vec3 irradiance = vec3(0.0);
    
    
    for (int i = 0; i < 9; ++i) {
        const float factor = getCosineLobeFactor(i);
        const float Ylm = harmonics(i, normal);
        const vec3 Llm = texelFetch(coefficients, ivec2(i,0), 0).rgb;
        irradiance += factor * Llm * Ylm;
    }
    
    
    
    FragColor = vec4(irradiance, 1.0);
}