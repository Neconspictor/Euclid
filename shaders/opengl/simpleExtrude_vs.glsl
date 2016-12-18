#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 transform;
uniform float extrudeValue;
void main()
{
    vec3 norm = normalize(normal);
    
    // extrude position on normal
    vec3 finalPosition = position + extrudeValue * norm; 
    gl_Position = transform * vec4(finalPosition, 1.0f);
} 