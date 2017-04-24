#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 modelView;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 biasMatrix;

out VS_OUT {
	vec3 fragPos;
	vec2 texCoords;
	vec3 normal;
	vec3 reflectPosition;
	vec4 fragPosLightSpace;
	mat3 TBN;
} vs_out;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
	vs_out.fragPos = vec3(model * vec4(position, 1.0f));
	vs_out.texCoords = texCoords;
    vs_out.normal = normalize(mat3(normalMatrix) * normal);
    //fragmentPosition = vec3(modelView * vec4(position, 1.0f));
    vs_out.reflectPosition = vec3(model * vec4(position, 1.0f));
	vs_out.fragPosLightSpace = biasMatrix * lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
	
	
	vec3 T = normalize(normalMatrix * normalize(tangent));
	vec3 N = normalize(vs_out.normal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = normalize(cross(T, N));
	vs_out.TBN = mat3(T, B, N);
} 