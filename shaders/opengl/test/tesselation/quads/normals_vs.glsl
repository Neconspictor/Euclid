#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 positionViewSpace;
} vs_out;

uniform mat4 modelView;
uniform mat4 transform;
uniform mat3 normalMatrix;

void main()
{
    gl_Position = transform * vec4(position, 1.0f); 
    vs_out.normal = normalize(normalMatrix * normal);
    vs_out.tangent = normalize(normalMatrix * tangent);
    vs_out.bitangent = normalize(normalMatrix * bitangent);
    vs_out.positionViewSpace = vec3(modelView * vec4(position, 1.0f)); 
}