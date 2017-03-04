#version 330 core

in vec3 fragPos;
out vec4 color;

uniform samplerCube cubeDepthMap;
uniform vec3 lightPos;
uniform float range;


void main()
{ 
		vec3 fragToLight = fragPos - lightPos;
		float closestDepth = texture(cubeDepthMap, fragToLight).r;
		float closestDepthRanged = closestDepth * range;
    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // Now test for shadows
    float bias = 0.05;
		currentDepth -= bias;
    float shadow = currentDepth > closestDepthRanged ? 1.0 : 0.0;
		
    color = vec4(vec3(1 - shadow), 1.0);
}