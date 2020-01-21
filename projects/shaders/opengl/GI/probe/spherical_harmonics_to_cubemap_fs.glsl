#version 460 core

in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

#include "util/spherical_harmonics.glsl"


layout (binding = 0) uniform sampler1DArray coefficients;

#ifndef SAMPLER_1D_SH
uniform int arrayIndex;
#endif

uniform int cubeMapSide;

#include "util/cubemap.glsl"


void main (void)
{
  const float gamma = 2.2f;  
  
  vec2 uv = Frag_UV;
  
   // convert optionally from left-handed to right handed
  #ifdef CONVERT_LH_TO_RH
	uv.x = 1.0 - uv.x;
	//uv.y = 1.0 - uv.y;
  #endif
  
 
  vec3 direction = normalize(getDirection(cubeMapSide, uv));
  
  
  
  vec4 irradiance = vec4(computeIrradiance(coefficients, arrayIndex, direction), 1.0);
  
  
  irradiance.rgb = pow(irradiance.rgb, vec3(1.0/gamma)); 
  
  Out_Color = Frag_Color * irradiance;
}