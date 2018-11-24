#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

struct DirLight {
    vec3 direction;
    vec3 color;
};

struct Material {
    sampler2D albedoMap;
	sampler2D aoMap;
	sampler2D metallicMap;
	sampler2D normalMap;
	sampler2D roughnessMap;
	sampler2D shadowMap;
};

uniform DirLight dirLight;
uniform Material material;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 modelView;

uniform mat4 eyeToLightSpaceMatrix;
uniform mat4 lightProjMatrix;
uniform mat4 lightViewMatrix;
uniform mat4 biasMatrix;
uniform mat4 view;

uniform vec3 cameraPos;

out VS_OUT {
	vec3 fragment_position_eye;
	vec2 tex_coords;
	vec3 normal_view;
	vec4 fragment_position_lightspace; // needed for shadow calculation
	mat3 TBN_eye_directions; // used to transform the reflection vector from tangent to eye space. 
						  //  This matrix mustn't be used with positions!!!
	vec3 light_direction_eye; // the light direction in tangent space	
} vs_out;

void main()
{

    gl_Position = transform * vec4(position, 1.0f);
	vs_out.fragment_position_eye = vec3(modelView * vec4(position, 1.0f));
	vs_out.tex_coords = texCoords;

	vec4 fragPosWorld = model * vec4(position, 1.0f);
	//vs_out.fragPosLightSpace = biasMatrix * lightSpaceMatrix * fragPosWorld;
	//vs_out.fragment_position_lightspace = biasMatrix * eyeToLightSpaceMatrix * vec4(vs_out.fragment_position_eye, 1.0f);
	vs_out.fragment_position_lightspace = biasMatrix * eyeToLightSpaceMatrix * fragPosWorld;
	
	
	mat3 normal_matrix = mat3(transpose(inverse(modelView)));
	vec3 normal_eye = normalize(normal_matrix * normal);
	vs_out.normal_view = normal_eye;
	vec3 tangent_eye = normalize(normal_matrix * tangent); // only normal is allowed to be multiplied by normal_matrix!
	
	float dotTN = dot(normal_eye, tangent_eye);
	
	if (dotTN < 0.0) {
		//world_tangent = -1.0 * world_tangent;
	};
	
	tangent_eye = normalize(tangent_eye - (dot(normal_eye, tangent_eye) * normal_eye));
	
	vec3 bitangent_eye = normalize(normal_matrix * bitangent);

	vs_out.TBN_eye_directions = mat3(tangent_eye, bitangent_eye, normal_eye);

	// create inverse TBN matrix to have a matrix that transforms from eye to tangent space
	//mat3 TBN_eye_inverse = inverse(vs_out.TBN_world_directions);

	//convert all needed vectors to tangent space
	// Note: TBN is mat3 since it doesn't have any translation; it is save to use it for positions, too.
	//vec3 view_direction_world = cameraPos - vs_out.fragment_position_world;
	vec3 light_direction_world = normalize(-dirLight.direction);	

	//vs_out.view_direction_tangent = TBN_eye_inverse * view_direction_world;
	vs_out.light_direction_eye = vec3(view * vec4(light_direction_world, 0));
	
	
	
	
	
	//normal offset shadow stuff
	
	//scale normal offset by shadow depth
	vec4 positionLightView = vs_out.fragment_position_lightspace;
	float shadowFOVFactor = max(lightProjMatrix[0].x, lightProjMatrix[1].y);
	vec2 size = textureSize(material.shadowMap, 0);
	float shadowMapTexelSize = 1 / max(size.x, size.y);
	
	shadowMapTexelSize *= abs(vs_out.fragment_position_lightspace.z) * shadowFOVFactor;
	
	vec4 positionLightSpace;
	float cosLightAngle = dot(vs_out.light_direction_eye, normal_eye);
	
	bool bNormalOffsetScale = true;
	float normalOffsetScale = bNormalOffsetScale ? clamp(1 - cosLightAngle, 0, 1) : 1.0;
	float shadowNormalOffset = 10;
	
	
	normalOffsetScale *= shadowNormalOffset * shadowMapTexelSize;
	vec4 shadowOffset = vec4(normal_eye * normalOffsetScale, 0);
	
	bool bOnlyUVNormalOffset = true;
	
	if (bOnlyUVNormalOffset) {
		positionLightSpace = vs_out.fragment_position_lightspace;
		
		vec4 shadowPositionEyeUVOnly = fragPosWorld  + shadowOffset;
		vec4 UVOffsetPositionLightSpace = biasMatrix * eyeToLightSpaceMatrix * shadowPositionEyeUVOnly;
		
		positionLightSpace.xy = UVOffsetPositionLightSpace.xy;
		
	} else {
		vec4 shadowPositionEye = fragPosWorld  + shadowOffset;
		positionLightSpace =  biasMatrix * eyeToLightSpaceMatrix * shadowPositionEye;
	}
	
	//vs_out.fragment_position_lightspace = positionLightSpace;
}
