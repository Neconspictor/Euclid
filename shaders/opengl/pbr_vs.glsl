#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;

struct DirLight {
    vec3 direction;
    vec3 color;
};

struct Material {
    sampler2D albedoMap;
	sampler2D aoMap;
	sampler2D metallicMap;
	sampler2D normalMap;
	sampler2D specularMap;
	sampler2D roughnessMap;
	sampler2D shadowMap;
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

uniform vec3 cameraPos;

out VS_OUT {
	vec3 fragPos;
	vec2 texCoords;
	vec4 fragPosLightSpace; // needed for shadow calculation
	vec3 TangentFragPos;
	mat3 TBN;
	vec3 T;
	vec3 B;
	vec3 N;
	vec3 lightDir;
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
	
	
	
	mat4 model4D = (inverse(modelView));
	vs_out.normal = normalize((model4D * vec4(normalNormalized, 1)).rgb);
	vs_out.normal = normalNormalized;

	mat3 model3D = mat3(model4D);
	
	vec3 N = normalize((model3D * normalNormalized).xyz); // original normalMatrix
	vec3 T = normalize(model3D * tangentNormalized);	// original normalMatrix
	
	float dotTN = dot(N, T);
	
	if (dotTN < 0.0) {
		//T = -1.0 * T;
	};
	
	T = normalize(T - (dot(N, T) * N));
	
	vec3 B = normalize(cross(N, T));

	mat3 TBN = transpose(mat3(T, B, N));
	
	
	
	vs_out.T = tangentNormalized;
	vs_out.B = B;
	vs_out.N = normalNormalized;
	vs_out.TBN = TBN;

	vs_out.TangentFragPos = TBN * vs_out.fragPos;
	
	
	vec3 pos = vec3(model *  vec4(position, 1));
	
	
	vec3 lightDir = normalize(-dirLight.direction);	
	vs_out.tangentLightDir = normalize(TBN * lightDir);
	vs_out.lightDir = lightDir;
	
	vec3 viewDir = normalize(cameraPos - pos);
	vs_out.tangentViewDir = normalize(TBN * viewDir);
	
	
	//normal offset shadow stuff
	
	//scale normal offset by shadow depth
	vec4 positionLightView = lightSpaceMatrix * fragPosWorld;
	float shadowFOVFactor = max(lightProjMatrix[0].x, lightProjMatrix[1].y);
	vec2 size = textureSize(material.shadowMap, 0);
	float shadowMapTexelSize = 1 / max(size.x, size.y);
	
	//shadowMapTexelSize *= abs(positionLightView.z) * shadowFOVFactor;
	
	vec4 positionLightSpace;
	vec3 toLight = lightDir;
	float cosLightAngle = dot(toLight, normalNormalized);
	
	bool bNormalOffsetScale = true;
	float normalOffsetScale = bNormalOffsetScale ? clamp(1 - cosLightAngle, 0, 1) : 1.0;
	float shadowNormalOffset = 2;
	
	
	normalOffsetScale *= shadowNormalOffset * shadowMapTexelSize;
	vec4 shadowOffset = vec4(normalNormalized.xyz * normalOffsetScale, 0);
	
	bool bOnlyUVNormalOffset = true;
	
	if (bOnlyUVNormalOffset) {
		positionLightSpace = lightSpaceMatrix * fragPosWorld;
		
		vec4 shadowPositionWorldUVOnly = fragPosWorld + shadowOffset;
		vec4 UVOffsetPositionLightSpace = lightSpaceMatrix * shadowPositionWorldUVOnly;
		
		positionLightSpace.xy = UVOffsetPositionLightSpace.xy;
		
	} else {
		vec4 shadowPositionWorld = fragPosWorld + shadowOffset;
		positionLightSpace =  lightSpaceMatrix * shadowPositionWorld;
	}
	
	vs_out.fragPosLightSpace = positionLightSpace;
} 