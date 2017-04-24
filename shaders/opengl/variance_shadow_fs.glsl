#version 330 core

// Output data
layout(location = 0) out vec2 moments;
//out vec3 moments;


float linearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
	float near_plane = 0.1;
	float far_plane = 12;
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{             
		 float depth = gl_FragCoord.z;
     float moment1 = depth;
		 float moment2 = moment1 * moment1;
		 
		 // Adjusting moments (this is sort of bias per pixel) using partial derivative
		float dx = dFdx(depth);
		float dy = dFdy(depth);
		moment2 += 0.25*(dx*dx+dy*dy);
		
		/*moments.r = moment1;
		moments.g = moment2;
		moments.b = moment1;
		moments.a = 1.0;*/
		moments = vec2(moment1, moment2);
}