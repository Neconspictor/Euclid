#include "interface/common_interface.h"

#ifdef __cplusplus
    namespace nex {
        namespace cluster {
#endif 


struct AABB
{
    NEX_VEC4 minView;
    NEX_VEC4 maxView;
    NEX_VEC4 minWorld;
    NEX_VEC4 maxWorld;
};

struct ActiveClusterConstants {
    NEX_UVEC4 numClusters; // cluster dimension in x,y and z axis; w component is unused
    NEX_VEC4 zReproductionConstants; // x: log(zFarDistance / zNearDistance), y: log(zNearDistance) * numClusters.z / log(zFarDistance/zNearDistance)
};


struct Constants {
    NEX_MAT4 invProj;
    NEX_MAT4 view;
    NEX_MAT4 invView;
    NEX_VEC4 zNearFar; // near and far plane in view space; z and w component unused
};

struct LightGrid
{
    NEX_UINT offset;
    NEX_UINT count;
};

#ifdef __cplusplus
        }
    }
#endif 