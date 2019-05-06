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

layout(binding = 0)uniform sampler2D heightMap;
uniform vec3 worldDimension;


vec3 samplePosition(vec2 coord) {
    const float x = coord.x * worldDimension.x - worldDimension.x/2.0; //
    const float y = worldDimension.y * texture(heightMap, coord).r; 
    const float z = -(coord.y * worldDimension.z - worldDimension.z/2.0); // - worldDimension.z/2.0
    
    return vec3(x, y, z);
}

vec3 calcNormalObjectSpace() {
    return vec3(0,1,0);
}

void main()
{
    const vec4 positionFinal = vec4(samplePosition(texCoords), 1.0);
    //const vec4 positionFinal = vec4(position, 1.0);
    gl_Position = transform * positionFinal;
    vs_out.normal = normalize(normalMatrix * normal);
    vs_out.tangent = normalize(normalMatrix * tangent);
    vs_out.bitangent = normalize(normalMatrix * bitangent);
    vs_out.positionViewSpace = vec3(modelView * positionFinal); 
}