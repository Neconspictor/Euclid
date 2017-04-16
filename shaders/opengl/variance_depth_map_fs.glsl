#version 330 core

in vec2 texCoordsFS;
out vec4 color;

uniform sampler2D vDepthMap;
uniform float near_plane;
uniform float far_plane;

float linearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{ 
		float moment1 = texture(vDepthMap, texCoordsFS).r;
    color = vec4(vec3(moment1), 1.0); // orthographic
		//color = vec4(vec3(linearizeDepth(moment1) / far_plane), 1.0); // perspective
}