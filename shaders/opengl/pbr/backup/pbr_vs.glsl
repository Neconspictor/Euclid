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

uniform mat4 lightSpaceMatrix;
uniform mat4 lightProjMatrix;
uniform mat4 lightViewMatrix;
uniform mat4 biasMatrix;
uniform mat4 view;

uniform vec3 cameraPos;

out VS_OUT {
	vec3 fragment_position_world;
	vec2 tex_coords;
	vec4 fragment_position_lightspace; // needed for shadow calculation
	mat3 TBN_world_directions; // used to transform the reflection vector from tangent to world space. 
						  //  This matrix mustn't be used with positions!!!
	vec3 view_direction_tangent; // the view direction in tangent space
	vec3 light_direction_tangent; // the light direction in tangent space	
} vs_out;

void main()
{

    gl_Position = transform * vec4(position, 1.0f);
	vs_out.fragment_position_world = vec3(model * vec4(position, 1.0f));
	vs_out.tex_coords = texCoords;

	vec4 fragPosWorld = model * vec4(position, 1.0f);
	//vs_out.fragPosLightSpace = biasMatrix * lightSpaceMatrix * fragPosWorld;
	vs_out.fragment_position_lightspace = lightSpaceMatrix * fragPosWorld;
	
	
	mat3 normal_matrix = mat3(transpose(inverse(model)));
	vec3 world_normal = normalize(normal_matrix * normal);
	//vec3 world_tangent = normalize(vec3(model * vec4(tangent, 0.0))); // only normal is allowed to be multiplied by normal_matrix!
	vec3 world_tangent = normalize(normal_matrix * tangent); // only normal is allowed to be multiplied by normal_matrix!
	
	float dotTN = dot(world_normal, world_tangent);
	
	if (dotTN < 0.0) {
		//world_tangent = -1.0 * world_tangent;
	};
	
	world_tangent = normalize(world_tangent - (dot(world_normal, world_tangent) * world_normal));
	
	//vec3 world_bitangent = normalize(cross(world_normal, world_tangent));
	vec3 world_bitangent = normalize(normal_matrix * bitangent);

	vs_out.TBN_world_directions = mat3(world_tangent, world_bitangent, world_normal);

	// create inverse TBN matrix to have a matrix that transforms from eye to tangent space
	mat3 TBN_eye_inverse = inverse(vs_out.TBN_world_directions);

	//convert all needed vectors to tangent space
	// Note: TBN is mat3 since it doesn't have any translation; it is save to use it for positions, too.
	vec3 view_direction_world = cameraPos - vs_out.fragment_position_world;
	vec3 light_direction_world = normalize(-dirLight.direction);	

	vs_out.view_direction_tangent = TBN_eye_inverse * view_direction_world;
	vs_out.light_direction_tangent = TBN_eye_inverse * light_direction_world;
	
	
	
	
	
	//normal offset shadow stuff
	
	//scale normal offset by shadow depth
	vec4 fragment_position_world = vec4(vs_out.fragment_position_world, 1.0);
	vec4 positionLightView = lightSpaceMatrix * fragment_position_world;
	float shadowFOVFactor = max(lightProjMatrix[0].x, lightProjMatrix[1].y);
	vec2 size = textureSize(material.shadowMap, 0);
	float shadowMapTexelSize = 1 / max(size.x, size.y);
	
	//shadowMapTexelSize *= abs(positionLightView.z) * shadowFOVFactor;
	
	vec4 positionLightSpace;
	float cosLightAngle = dot(light_direction_world, world_normal);
	
	bool bNormalOffsetScale = true;
	float normalOffsetScale = bNormalOffsetScale ? clamp(1 - cosLightAngle, 0, 1) : 1.0;
	float shadowNormalOffset = 10;
	
	
	normalOffsetScale *= shadowNormalOffset * shadowMapTexelSize;
	vec4 shadowOffset = vec4(world_normal * normalOffsetScale, 0);
	
	bool bOnlyUVNormalOffset = true;
	
	if (bOnlyUVNormalOffset) {
		positionLightSpace = lightSpaceMatrix * fragment_position_world;
		
		vec4 shadowPositionWorldUVOnly = fragment_position_world + shadowOffset;
		vec4 UVOffsetPositionLightSpace = lightSpaceMatrix * shadowPositionWorldUVOnly;
		
		positionLightSpace.xy = UVOffsetPositionLightSpace.xy;
		
	} else {
		vec4 shadowPositionWorld = fragment_position_world + shadowOffset;
		positionLightSpace =  lightSpaceMatrix * shadowPositionWorld;
	}
	
	vs_out.fragment_position_lightspace = positionLightSpace;
} 