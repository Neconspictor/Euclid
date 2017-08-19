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

uniform vec3 viewPos;

out VS_OUT {
	vec3 fragPos;
	vec2 texCoords;
	vec4 fragPosLightSpace; // needed for shadow calculation
	vec3 TangentLightPos;
    vec3 TangentViewPos;
	vec3 TangentFragPos;
	mat3 TBN;
} vs_out;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
	vs_out.fragPos = vec3(model * vec4(position, 1.0f));
	vs_out.texCoords = texCoords;
	
    //vs_out.normal = normalize(mat3(normalMatrix) * normal);
	//vs_out.normal = normal;
    //fragmentPosition = vec3(modelView * vec4(position, 1.0f));
	vs_out.fragPosLightSpace = biasMatrix * lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
	
	
	vec3 T = normalize(normalMatrix * tangent);	// original normalMatrix
	vec3 N = normalize(normalMatrix * normal); // original normalMatrix
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(T, N);
	
	// TBN must form a right handed coord system.
    // Some models have symetric UVs. Check and fix.
    if (dot(cross(N, T), B) < 0.0)
                T = T * -1.0;
	
	mat3 TBN = mat3(T, B, N);
	
	vs_out.TangentLightPos = TBN * vec3(100.0, 100.0, 100.0);
    vs_out.TangentViewPos  = TBN * viewPos;
	vs_out.TangentFragPos = TBN * vs_out.fragPos;
	vs_out.TBN = TBN;
} 