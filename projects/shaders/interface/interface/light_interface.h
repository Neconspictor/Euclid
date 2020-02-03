#ifndef LIGHT_INTERFACE_H
#define LIGHT_INTERFACE_H
#include "interface/common_interface.h"

#ifdef __cplusplus
    namespace nex {
#endif 

//IMPORTANT: Use std430 layout for all !

/**
 * Note: struct alignment: 64 bytes
 */
struct DirLight {
	NEX_VEC4 directionWorld; //xyz used
	NEX_VEC4 directionEye; //xyz used
	NEX_VEC4 color; //xyz used
    float power;
    #ifdef __cplusplus
    float _pad[3];
    #endif
};

/**
 * Note: struct alignment: 16 bytes 
 */
struct EnvironmentLight
{
	NEX_VEC4 position;
	NEX_VEC4 minWorld;
	NEX_VEC4 maxWorld;
	float sphereRange;
	NEX_UINT enabled;
	NEX_UINT usesBoundingBox; // specifies whether to use AABB or Sphere volume
	NEX_UINT arrayIndex;
};

/**
 * Note: struct alignment: 16 bytes 
 */
struct PointLight
{
    NEX_VEC4 position;
    NEX_VEC4 color;
    NEX_UINT enabled;
    float intensity;
    float sphereRange;
    #ifdef __cplusplus
    float _pad[1];
    #endif
};


#ifdef __cplusplus
    }
#endif 
#endif