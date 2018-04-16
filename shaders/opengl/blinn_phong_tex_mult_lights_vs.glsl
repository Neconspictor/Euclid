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

struct Material {
    sampler2D diffuseMap;
    sampler2D emissionMap;
    sampler2D normalMap;
	sampler2D reflectionMap;
    sampler2D specularMap;
	sampler2D shadowMap;
	sampler2D vsMap;
    float shininess;
};

uniform DirLight dirLight;
uniform Material material;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 modelView;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 lightProjMatrix;
uniform mat4 lightViewMatrix;
uniform mat4 biasMatrix;
uniform mat4 view;

uniform vec3 viewPos;

out VS_OUT {
	vec3 fragPos;
	vec2 texCoords;
	vec4 fragPosLightSpace; // needed for shadow calculation
	vec3 TangentFragPos;
	mat3 TBN;
	vec3 T;
	vec3 B;
	vec3 N;
	vec3 tangentLightDir;
	vec3 tangentViewDir;
	vec3 normal;
} vs_out;

void main()
{

	vec3 normalNormalized = normalize(normal);
	vec3 tangentNormalized = normalize(tangent);

	
    gl_Position = transform * vec4(position, 1.0f);
	vs_out.fragPos = vec3(model * vec4(position, 1.0f));
	vs_out.texCoords = texCoords;
	

	vec4 fragPosWorld = model * vec4(position, 1.0f);
	//vs_out.fragPosLightSpace = biasMatrix * lightSpaceMatrix * fragPosWorld;
	vs_out.fragPosLightSpace = lightSpaceMatrix * fragPosWorld;
	
	mat3 modelView3D = mat3(model);
	//mat3 modelView3D = mat3(model);
	modelView3D = normalMatrix;
	//modelView3D = transpose(inverse(mat3(model)));
	
	
	vs_out.normal = normalize(modelView3D * normalNormalized);
	
	vec3 N = normalize((modelView3D * normalNormalized).xyz); // original normalMatrix
	vec3 T = normalize((modelView3D * tangentNormalized).xyz);	// original normalMatrix
	
	float dotTN = dot(N, T);
	
	if (dotTN < 0.0) {
		//T = -1.0 * T;
	};
	
	T = normalize(T - (dot(N, T) * N));
	
	vec3 B = normalize(cross(N, T));
	
	vs_out.T = tangentNormalized;
	vs_out.B = B;
	vs_out.N = normalNormalized;
	
	
	mat3 TBN = transpose(mat3(T, B, N));
	vs_out.TBN = TBN;

	vs_out.TangentFragPos = TBN * vs_out.fragPos;
	
	
	vec3 pos = vec3(model *  vec4(position, 1));
	
	
	vec3 viewLightDir = normalize(-dirLight.direction);	
	vs_out.tangentLightDir = normalize(viewLightDir.xyz);
	
	vec3 viewDir = normalize(viewPos - pos);
	vs_out.tangentViewDir = normalize(viewDir);
	//vs_out.tangentViewDir = normalize((vec4(viewDir,0)).xyz);
	
	
	//normal offset shadow stuff
	
	//scale normal offset by shadow depth
	vec4 positionLightView = lightViewMatrix * model * vec4(position, 1);
	float shadowFOVFactor = max(lightProjMatrix[0].x, lightProjMatrix[1].y);
	vec2 size = textureSize(material.shadowMap, 0);
	float shadowMapTexelSize = 1 / min(size.x, size.y);
	shadowMapTexelSize *= abs(positionLightView.z) * shadowFOVFactor;
	
	vec4 positionLightSpace;
	vec3 toLight = vs_out.tangentLightDir;
	float cosLightAngle = dot(toLight, normalNormalized);
	
	bool bNormalOffsetScale = true;
	float normalOffsetScale = bNormalOffsetScale ? clamp(1 - cosLightAngle, 0, 1) : 1.0;
	float shadowNormalOffset = 5.0;
	normalOffsetScale *= shadowNormalOffset * shadowMapTexelSize;
	vec4 shadowOffset = vec4(normalNormalized.xyz * normalOffsetScale, 0);
	
	bool bOnlyUVNormalOffset = true;
	
	if (bOnlyUVNormalOffset) {
		positionLightSpace = lightSpaceMatrix * vec4(pos, 1);
		
		vec4 shadowPositionWorldUVOnly = vec4(pos, 1) + shadowOffset;
		vec4 UVOffsetPositionLightSpace = lightSpaceMatrix * shadowPositionWorldUVOnly;
		
		positionLightSpace.xy = UVOffsetPositionLightSpace.xy;
		
	} else {
		vec4 shadowPositionWorld = vec4(pos, 1) + shadowOffset;
		positionLightSpace =  lightSpaceMatrix * shadowPositionWorld;
	}
} 