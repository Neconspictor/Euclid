#version 330 core

// Output data
layout(location = 0) out float fragmentdepth;

float linearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
	float near_plane = 0.1;
	float far_plane = 12;
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{             
     //fragmentdepth = linearizeDepth(gl_FragCoord.z);
	 fragmentdepth = gl_FragCoord.z;
}  