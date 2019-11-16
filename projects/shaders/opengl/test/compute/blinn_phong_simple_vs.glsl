#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

struct DirLight {
    vec3 direction;
};

uniform DirLight dirLight;

uniform mat4 transform;
uniform mat4 model;

uniform vec3 viewPos_world;

out VS_OUT {
    vec3 lightDirectionWorld;
    vec3 normalWorld;
    vec3 viewDirectionWorld;

	vec3 view_direction_tangent; // the view direction in tangent space
	vec3 light_direction_tangent; // the light direction in tangent space
    
    vec3 fragPosWorld;
    
    mat3 TBN;
    mat3 TBN_Inverse;
} vs_out;

void main()
{	
    gl_Position = transform * vec4(position, 1.0f);
	vec4 fragment_position_world = model * vec4(position, 1.0f);
    vs_out.fragPosWorld = fragment_position_world.rgb;
    
    
    mat3 normalMatrix = mat3(transpose(inverse(model)));
	vec3 normal_world = normalize(normalMatrix * normal);
	vec3 tangent_world = normalize(normalMatrix * tangent);
	tangent_world = normalize(tangent_world - (dot(normal_world, tangent_world) * normal_world));
	
	vec3 bitangent_world = normalize(normalMatrix * bitangent);
	
	float dotTN = dot(normal_world, tangent_world);
	
	// TBN must form a right handed coord system.
    // Some models have symetric UVs. Check and fix.
    if (dot(cross(normal_world, tangent_world), bitangent_world) < 0.0)
        tangent_world = tangent_world * -1.0;

	// create TBN matrix
	// for TBN we can ignore translations (this is only valid for direction vectors and not for positions!!!)
	mat3 TBN_world = mat3(tangent_world, bitangent_world, normal_world);


	// create inverse TBN matrix to have a matrix that transforms from eye to tangent space
	mat3 TBN_inverse_world = inverse(TBN_world);

	//convert all needed vectors to tangent space
	// Note: TBN is mat3 since it doesn't have any translation; it is save to use it for positions, too.
	
	vec3 view_direction_world = viewPos_world - fragment_position_world.rgb;
	vec3 light_direction_world = normalize(dirLight.direction);	

	vs_out.view_direction_tangent = TBN_inverse_world * normalize(view_direction_world);
	vs_out.light_direction_tangent = TBN_inverse_world * normalize(light_direction_world);
    
    
    
    vs_out.lightDirectionWorld = light_direction_world;
    vs_out.normalWorld = normal_world;
    vs_out.viewDirectionWorld = view_direction_world;
    
    vs_out.TBN = TBN_world;
    vs_out.TBN_Inverse = TBN_inverse_world;
} 