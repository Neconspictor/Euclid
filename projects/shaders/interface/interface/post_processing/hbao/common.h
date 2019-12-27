#ifndef HBAO_INTERFACE
#define HBAO_INTERFACE

#include "interface/common_interface.h"

#ifdef __cplusplus
    namespace nex {
#endif


	

	NEX_CONST_INT HBAO_RANDOMTEX_SIZE = 4;
	NEX_CONST_INT HBAO_RANDOM_ELEMENTS = HBAO_RANDOMTEX_SIZE * HBAO_RANDOMTEX_SIZE;


#ifndef __cplusplus
	/**
	 * NOTE: Shader code depends on float constant!
	 */

#ifndef HBAO_NUM_DIRECTIONS
#define HBAO_NUM_DIRECTIONS 16.0
#endif

#ifndef HBAO_NUM_STEPS
#define HBAO_NUM_STEPS 8.0
#endif

#endif

	

	struct SceneData {
	  NEX_MAT4  viewProjMatrix;
	  NEX_MAT4  viewMatrix;
	  NEX_MAT4  viewMatrixIT;
	  
	  NEX_UVEC2 viewport;
	  NEX_UVEC2 _pad;
	};

	struct HBAOData {
	  float   		RadiusToScreen;        	// radius
	  float   		R2;     				// 1/radius
	  float   		NegInvR2;     			// radius * radius
	  float   		NDotVBias;
	 
	  NEX_VEC2    	InvFullResolution;
	  NEX_VEC2    	InvQuarterResolution;
	  
	  float   		AOMultiplier;
	  float   		PowExponent;
	  NEX_VEC2    	_pad0;
	  
	  NEX_VEC4    	projInfo;
	  NEX_VEC2    	projScale;
	  int     		projOrtho;
	  int     		_pad1;
	  
	  NEX_VEC4    	float2Offsets[HBAO_RANDOM_ELEMENTS];
	  NEX_VEC4    	jitters[HBAO_RANDOM_ELEMENTS];
	};

#ifdef __cplusplus
    }
#endif 

#endif // HBAO_INTERFACE