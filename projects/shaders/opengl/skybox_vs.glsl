#version 330 core
layout (location = 0) in vec3 position;

smooth out vec3 texCoordsFS;

uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    /*mat4 inverseProjection = inverse(projection);
	mat3 inverseView = transpose(mat3(view));
		
	vec3 unprojected = (inverseProjection * vec4(position, 1.0)).xyz;
	vec3 wcNormal = inverseView * unprojected;

	//gl_Position = vec4(position, 1.0);
	gl_Position =  projection * view * vec4(position, 1.0);
	texCoordsFS = wcNormal;*/
	
	texCoordsFS = position;  
    gl_Position =  projection * view * vec4(position, 1.0);
} 