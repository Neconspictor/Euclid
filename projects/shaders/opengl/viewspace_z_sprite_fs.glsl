#version 460 core

in vec2 texCoordsFS;
out vec4 color;

uniform sampler2D sprite;

uniform float cameraRangeDistance;

void main()
{ 
	float viewspaceZ = vec3(texture(sprite, texCoordsFS)).r;
	
	//normalize it into range [0, 1]
	float linearDepth = -(viewspaceZ /= cameraRangeDistance);
    color = vec4(vec3(linearDepth), 1.0);
}