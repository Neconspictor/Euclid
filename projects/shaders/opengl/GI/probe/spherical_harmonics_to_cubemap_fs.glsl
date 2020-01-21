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
uniform mat4 cubeMapSideView;


vec3 mapAxis(int side) {

	switch(side) {
		case 0:
			return vec3(1,0,0);
		case 1:
		return vec3(-1,0,0);
		case 2:
		return vec3(0,1,0);
		case 3:
		return vec3(0,-1,0);
		case 4:
		return vec3(0,0,1);
		case 5:
		return vec3(0,0,-1);
	}
}


vec3 getUp(int side) {

	switch(side) {
		case 0:
			return vec3(0,-1,0);
		case 1:
		return vec3(0,-1,0);
		case 2:
		return vec3(0,0,1);
		case 3:
		return vec3(0,0,-1);
		case 4:
		return vec3(0,-1,0);
		case 5:
		return vec3(0,-1,0);
	}
}

void main (void)
{
  vec2 uv = Frag_UV;
  const float maxAngle = PI / 4.0;
  
  // spherical coordinates
  //uv.y = 1.0 - uv.y;
  //uv.x = 1.0 - uv.x;
  float theta = ((uv.y) * 2.0 - 1.0) * maxAngle - 1.0 * PI/2.0;
  float phi = (uv.x * 2.0 - 1.0) * maxAngle - PI/2.0;
  
  //make cartesian
  vec3 direction = vec3(0.0f);
  direction.x = sin(theta) * cos(phi);
  direction.z = cos(theta);
  direction.y = sin(theta) * sin(phi);
  
  direction.x = sin(theta) * cos(phi);
  direction.y = cos(theta);
  direction.z = sin(theta) * sin(phi);
  
 // direction = normalize(direction);
  direction = normalize(vec3(cubeMapSideView * vec4(direction, 0)));
  //rotate cartesian direction vector to cubemap side

  
  vec3 axis = mapAxis(cubeMapSide);
  vec3 up = getUp(cubeMapSide);
  vec3 right = normalize(cross(axis, up));
  
  vec4 irradiance = vec4(computeIrradiance(coefficients, arrayIndex, direction), 1.0);
  
  Out_Color = Frag_Color * irradiance;
}