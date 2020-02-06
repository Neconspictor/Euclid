#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#include "interface/buffers.h"

out VS_OUT {
    vec3 fragment_position_eye;
	vec3 normalEye;
    vec2 texCoords;
} vs_out;

void main() {
     gl_Position = objectData.transform * vec4(position, 1.0f);
	
    vs_out.fragment_position_eye = vec3(objectData.modelView * vec4(position, 1.0f));
    vs_out.texCoords = texCoords;    
    mat3 normalMatrix = mat3(inverse(transpose(objectData.modelView)));
	vs_out.normalEye = normalize(normalMatrix * normal);
}