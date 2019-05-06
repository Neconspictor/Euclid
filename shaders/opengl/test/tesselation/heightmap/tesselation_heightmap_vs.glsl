#version 450
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec2 texCoord_tcs_in;

uniform mat4 transform;

layout(binding = 0)uniform sampler2D heightMap;
uniform vec3 worldDimension;


vec3 samplePosition(vec2 coord) {
    const float x = coord.x * worldDimension.x - worldDimension.x/2.0; //
    const float y = worldDimension.y * texture(heightMap, coord).r; 
    const float z = -(coord.y * worldDimension.z - worldDimension.z/2.0); // - worldDimension.z/2.0
    
    return vec3(x, y, z);
}

void main() { 
  const vec4 positionFinal = vec4(samplePosition(texCoords), 1.0);
  gl_Position = transform * positionFinal;
  
  texCoord_tcs_in = texCoords;
}