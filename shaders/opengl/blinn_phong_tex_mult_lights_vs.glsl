#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;

struct DirLight {
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

uniform DirLight dirLight;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 modelView;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 biasMatrix;
uniform mat4 view;

uniform vec3 viewPos;

out VS_OUT {
	vec3 fragPos;
	vec2 texCoords;
	vec4 fragPosLightSpace; // needed for shadow calculation
	vec3 TangentFragPos;
	mat3 TBN;
	vec3 tangentLightDir;
	vec3 tangentViewDir;
	vec3 viewLightDir;
} vs_out;

void main()
{
    gl_Position = transform * vec4(position, 1.0f);
	vs_out.fragPos = vec3(model * vec4(position, 1.0f));
	vs_out.texCoords = texCoords;
	
    //vs_out.normal = normalize(normalMatrix * normal);
	//vs_out.normal = normal;
    //fragmentPosition = vec3(modelView * vec4(position, 1.0f));
	vec4 fragPosWorld = model * vec4(position, 1.0f);
	vs_out.fragPosLightSpace = biasMatrix * lightSpaceMatrix * fragPosWorld;
	
	
	mat3 modelView3D = mat3(transpose(inverse(model)));
	
	vec3 N = normalize(modelView3D * normal); // original normalMatrix
	vec3 T = normalize(modelView3D * tangent);	// original normalMatrix
	//T = normalize(T - dot(T, N) * N);
	vec3 B = normalize(cross(N, T));
	
	//mat3 TBN = mat3(T, B, N);
	mat3 TBN = mat3(
	T.x, B.x, N.x,
	T.y, B.y, N.y,
	T.z, B.z, N.z ) ;
	
	
	vs_out.TangentFragPos = TBN * vs_out.fragPos;
	vs_out.TBN = TBN;
	
	vs_out.viewLightDir = -dirLight.direction;
	
	vs_out.tangentLightDir = normalize(TBN * vs_out.viewLightDir);
	vec3 pos = vec3(model * vec4(position, 1));
	vs_out.tangentViewDir = normalize(TBN * (viewPos-pos));
} 