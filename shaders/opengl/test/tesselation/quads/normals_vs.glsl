#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 transform;
uniform mat3 normalMatrix;

void main()
{
    gl_Position = transform * vec4(position, 1.0f); 
    vs_out.normal = normalize(normalMatrix * normal);
}