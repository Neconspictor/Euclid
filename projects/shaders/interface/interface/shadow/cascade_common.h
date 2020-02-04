#ifndef CASCADE_COMMON_INTERFACE_H
#define CASCADE_COMMON_INTERFACE_H
#include "interface/common_interface.h"

#ifdef __cplusplus
namespace nex {
#endif 

#ifndef CSM_MAX_NUM_CASCADES
#define CSM_MAX_NUM_CASCADES 10
#endif

	struct CascadeData {
		NEX_MAT4 lightViewProjectionMatrices[CSM_MAX_NUM_CASCADES];
		NEX_VEC4 scaleFactors[CSM_MAX_NUM_CASCADES];
		NEX_VEC4 cascadedSplits[CSM_MAX_NUM_CASCADES];
	};

#ifdef __cplusplus
}
#endif 

#endif //CASCADE_COMMON_INTERFACE_H