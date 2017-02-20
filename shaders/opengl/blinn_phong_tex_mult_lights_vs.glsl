#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 modelView;
uniform mat4 normalMatrix;
uniform mat4 lightSpaceMatrix;

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec3 reflectPosition;
	vec4 fragPosLightSpace;
} vs_out;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
    vs_out.normal = mat3(normalMatrix) * normal;
    //fragmentPosition = vec3(modelView * vec4(position, 1.0f));
    vs_out.fragPos = vec3(model * vec4(position, 1.0f));
    vs_out.reflectPosition = vec3(model * vec4(position, 1.0f));
    vs_out.texCoords = texCoords;
		vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
} 