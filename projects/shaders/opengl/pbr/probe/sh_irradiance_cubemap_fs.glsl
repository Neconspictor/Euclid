#version 460 core

out vec4 FragColor;
in vec3 localPos;

//#define SAMPLER_1D_SH
#include "util/spherical_harmonics.glsl"


layout (binding = 0) uniform SAMPLER_TYPE_SH coefficients;

#ifndef SAMPLER_1D_SH
uniform int arrayIndex;
#endif


void main()
{		    
    vec3 normal = normalize(localPos);
	
	#ifdef SAMPLER_1D_SH
		vec3 irradiance = computeIrradiance(coefficients, normal);
	#else
		vec3 irradiance = computeIrradiance(coefficients, arrayIndex, normal);
	#endif
	
    FragColor = vec4(irradiance, 1.0);
}