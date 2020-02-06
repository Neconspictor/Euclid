#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout(std140, binding = 0) buffer TransformBuffer {
    mat4 model;
    mat4 transform;
    mat4 prevTransform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;

out VS_OUT {
    vec3 fragment_position_eye;
	vec3 normalEye;
    vec2 texCoords;
} vs_out;

void main() {
     gl_Position = transforms.transform * vec4(position, 1.0f);
	
    vs_out.fragment_position_eye = vec3(transforms.modelView * vec4(position, 1.0f));
    vs_out.texCoords = texCoords;    
    mat3 normalMatrix = mat3(inverse(transpose(transforms.modelView)));
	vs_out.normalEye = normalize(normalMatrix * normal);
}