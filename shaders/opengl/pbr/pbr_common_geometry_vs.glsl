layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout(binding = 0) buffer TransformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 transform;
    mat4 prevTransform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;

out VS_OUT {
	//vec4 fragment_position_eye;
    //float viewSpaceZ;
    vec4 position_ndc;
    vec4 position_ndc_previous;
	vec2 tex_coords;
	mat3 TBN_eye_directions; // used to transform the normal vector from tangent to eye space. 
						  //  This matrix mustn't be used with positions!!!	
} vs_out;

void commonVertexShader() {
    gl_Position = transforms.transform * vec4(position, 1.0f);
	
    vs_out.position_ndc = gl_Position;
    vs_out.position_ndc_previous = transforms.prevTransform *  vec4(position, 1.0f);
    
    vs_out.tex_coords = texCoords;
    
    mat3 normalMatrix = mat3(inverse(transpose(transforms.modelView)));
	
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