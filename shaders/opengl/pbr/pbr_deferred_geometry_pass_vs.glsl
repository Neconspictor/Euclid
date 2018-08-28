#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;


uniform mat4 transform;
uniform mat4 model;
uniform mat4 modelView;
uniform mat3 modelView_normalMatrix;
uniform mat4 eyeToLightSpaceMatrix;
uniform mat4 biasMatrix;

out VS_OUT {
	vec4 fragment_position_eye;
	vec2 tex_coords;
	mat3 TBN_eye_directions; // used to transform the normal vector from tangent to eye space. 
						  //  This matrix mustn't be used with positions!!!	
} vs_out;

void main()
{

    gl_Position = transform * vec4(position, 1.0f);
	vs_out.fragment_position_eye = modelView * vec4(position, 1.0f);
	vs_out.tex_coords = texCoords;
	
	mat3 normalMatrix = mat3(inverse(transpose(modelView)));
	
	vec3 normal_eye = normalize(normalMatrix * normal);
	vec3 tangent_eye = normalize(normalMatrix * tangent);
	tangent_eye = normalize(tangent_eye - (dot(normal_eye, tangent_eye) * normal_eye));
	
	vec3 bitangent_eye = normalize(normalMatrix * bitangent);
	
	float dotTN = dot(normal_eye, tangent_eye);
	
	// TBN must form a right handed coord system.
    // Some models have symetric UVs. Check and fix.
    if (dot(cross(normal_eye, tangent_eye), bitangent_eye) < 0.0)
        tangent_eye = tangent_eye * -1.0;

	vs_out.TBN_eye_directions = mat3(tangent_eye, bitangent_eye, normal_eye);
}