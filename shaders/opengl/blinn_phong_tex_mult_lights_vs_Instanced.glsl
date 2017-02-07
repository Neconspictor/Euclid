#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in mat4 instancedModel;

uniform mat4 projection;
uniform mat4 view;

out vec3 fragmentPosition;
out vec3 normalVS;
out vec2 texCoordsFS;
out vec3 reflectPosition;

void main()
{
    gl_Position = projection * view * instancedModel * vec4(position, 1.0f);
    mat4 normalMatrix = transpose(inverse(instancedModel));
    normalVS = mat3(normalMatrix) * normal;
    //fragmentPosition = vec3(view * instancedModel * vec4(position, 1.0f));
    fragmentPosition = vec3(view * instancedModel * vec4(position, 1.0f));
    reflectPosition = vec3(instancedModel * vec4(position, 1.0f));
    texCoordsFS = texCoords;
} 