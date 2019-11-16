#version 430
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;


uniform mat4 transform;

void main()
{	
    gl_Position = transform * vec4(position, 1.0f);
} 