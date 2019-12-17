#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 transform;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main()
{
    gl_Position = transform * vec4(position, 1.0f); 
    //vs_out.normal = normalize(vec3(projection * vec4(normalMatrix * normal, 1.0)));
    vs_out.normal = normalize(normalMatrix * normal);
}