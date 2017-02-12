#version 330 core
#define M_PI 3.1415926535897932384626433832795
#define M_TWO_PI 6.283185307179586476925286766559
in vec3 texCoordsFS;
out vec4 color;

uniform sampler2D panorama;

vec2 envMapEquirect(vec3 wcNormal, float flipEnvMap) {
  float phi = acos(wcNormal.y);
  float theta = atan(flipEnvMap * wcNormal.x, wcNormal.z) + M_PI;
  return vec2(theta / M_TWO_PI, phi / M_PI);
}

vec2 envMapEquirect(vec3 wcNormal) {
    //-1.0 for left handed coordinate system oriented texture (usual case)
    return envMapEquirect(wcNormal, -1.0);
}


void main()
{    
    color = texture(panorama, envMapEquirect(normalize(texCoordsFS)));
}