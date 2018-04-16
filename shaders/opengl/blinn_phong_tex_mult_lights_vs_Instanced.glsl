#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in mat4 instancedModel;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 lightProjMatrix;
uniform mat4 lightSpaceMatrix;


out VS_OUT {
	vec3 fragPos;
	vec2 texCoords;
	vec4 fragPosLightSpace; // needed for shadow calculation
	vec3 TangentFragPos;
	mat3 TBN;
	vec3 tangentLightDir;
	vec3 tangentViewDir;
} vs_out;

void main()
{
    gl_Position = projection * view * instancedModel * vec4(position, 1.0f);
    mat4 normalMatrix = transpose(inverse(instancedModel));
    //vs_out.normal = mat3(normalMatrix) * normal;
    //vs_out.fragPos = vec3(view * instancedModel * vec4(position, 1.0f));
		vs_out.fragPos = vec3(instancedModel * vec4(position, 1.0f));
    //vs_out.reflectPosition = vec3(instancedModel * vec4(position, 1.0f));
    vs_out.texCoords = texCoords;
		vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
} 