#version 330 core
layout (location = 0) in vec3 position;

out vec3 texCoordsFS;

uniform mat4 transform;

void main()
{
    vec3 normalized = normalize(position);
    vec4 pos =   transform * vec4(normalized, 1.0);
    pos.z = pos.w;
    gl_Position = pos.xyzw;
    texCoordsFS = normalized;
} 