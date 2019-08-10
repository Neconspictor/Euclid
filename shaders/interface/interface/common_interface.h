#ifndef SHADER_INTERFACE_MATH_MACROS

#define SHADER_INTERFACE_MATH_MACROS 1

    #ifdef __cplusplus
		#define NEX_UINT glm::uint
		#define NEX_UVEC2 glm::uvec2
		#define NEX_UVEC3 glm::uvec3
		#define NEX_UVEC4 glm::uvec4		
		#define NEX_VEC2 glm::vec2
		#define NEX_VEC3 glm::vec3
		#define NEX_VEC4 glm::vec4
		#define NEX_MAT2 glm::mat2
		#define NEX_MAT3 glm::mat3
		#define NEX_MAT4 glm::mat4
    #else 
		#define NEX_UINT uint
		#define NEX_UVEC2 uvec2
		#define NEX_UVEC3 uvec3
		#define NEX_UVEC4 uvec4		
		#define NEX_VEC2 vec2
		#define NEX_VEC3 vec3
		#define NEX_VEC4 vec4
		#define NEX_MAT2 mat2
		#define NEX_MAT3 mat3
		#define NEX_MAT4 mat4
    #endif

#endif