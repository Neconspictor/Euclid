#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 modelView;
uniform mat4 normalMatrix;

out vec3 fragmentPosition;
out vec3 normalVS;
out vec2 texCoordsFS;
out vec3 reflectPosition;



void main()
{
    gl_Position = transform * vec4(position, 1.0f);
    //normalVS = mat3(normalMatrix) * normal;
    normalVS = normal;
    //fragmentPosition = vec3(modelView * vec4(position, 1.0f));
    //fragmentPosition = vec3(modelView * vec4(position, 1.0f));
    fragmentPosition = position;
    //reflectPosition = vec3(model * vec4(position, 1.0f));
    reflectPosition = vec3(gl_Position);
    texCoordsFS = texCoords;
} 