#include "interface/common_interface.h"

#ifdef __cplusplus
    namespace nex {
#endif 


struct DirLight {
	NEX_VEC3 directionWorld;
	NEX_VEC3 directionEye;
	NEX_VEC3 color;
    float power;
};

struct EnvironmentLight
{
	NEX_VEC4 position;
	NEX_VEC4 minWorld;
	NEX_VEC4 maxWorld;
	float sphereRange;
	NEX_UINT enabled;
	NEX_UINT usesBoundingBox; // specifies whether to use AABB or Sphere volume
};


#ifdef __cplusplus
    }
#endif 