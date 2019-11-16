#include "interface/common_interface.h"

#ifdef __cplusplus
    namespace nex {
#endif 

//IMPORTANT: Use std430 layout for all !

/**
 * Note: struct alignment: 9 bytes 
 */
struct DirLight {
	NEX_VEC3 directionWorld;
	NEX_VEC3 directionEye;
	NEX_VEC3 color;
    float power;
    #ifdef __cplusplus
    float _pad[2];
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