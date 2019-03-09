#ifndef CASCADE_COMMON_INCLUDE
#define CASCADE_COMMON_INCLUDE

#ifndef CSM_NUM_CASCADES
#define CSM_NUM_CASCADES 4
#endif

struct CascadeData {
	//mat4 viewMatrix;
	mat4 inverseViewMatrix;
	mat4 lightViewProjectionMatrices[CSM_NUM_CASCADES];
    vec4 scaleFactors[CSM_NUM_CASCADES];
	vec4 cascadedSplits[CSM_NUM_CASCADES];
};

#endif //CASCADE_COMMON_INCLUDE