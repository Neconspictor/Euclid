#version 450
layout (location = 0) in vec3 position;
/*layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;*/


out VS_OUT {
    vec3 normal;
    //vec2 texCoords;
} vs_out;

uniform mat4 transform;
uniform mat3 normalMatrix;

void main() { 

    //if (gl_VertexID == 1) {
    //     gl_Position = transform * vec4(position.x, 0.4, position.z, 1.0);  
    //} else {
        gl_Position = transform * vec4(position, 1.0);  
    //}
    
  //vs_out.normal = normalize(normalMatrix * normal);
  vs_out.normal = normalize(normalMatrix * vec3(0, 1, 0));
  //vs_out.texCoords = texCoords;
}