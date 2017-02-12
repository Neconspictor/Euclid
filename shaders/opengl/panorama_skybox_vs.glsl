#version 330 core
layout (location = 0) in vec3 position;

smooth out vec3 texCoordsFS;

uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    /*vec3 normalized = normalize(position);
    vec4 pos =   transform * vec4(normalized, 1.0);
    //pos.z = pos.w;
    gl_Position = pos.xyzw;
    texCoordsFS = position;*/
		
		mat4 inverseProjection = inverse(projection);
		mat3 inverseView = transpose(mat3(view));
		
		vec3 unprojected = (inverseProjection * vec4(position, 1.0)).xyz;
		vec3 wcNormal = inverseView * unprojected;
		
    vec3 normalized = normalize(position.xyz);
    vec4 pos =   transform * vec4(normalized, 1.0);
		//pos.z = pos.w;
		// flip x axis as most skybox textures are made for left handed coordinate systems 
		normalized.x = -normalized.x;

    //gl_Position = pos;
		gl_Position = vec4(position, 1.0);
    //texCoordsFS = normalized;
		texCoordsFS = wcNormal;
} 