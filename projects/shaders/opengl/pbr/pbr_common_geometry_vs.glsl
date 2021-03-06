#ifndef BONE_ANIMATION
#define BONE_ANIMATION 0
#endif


#if BONE_ANIMATION

#ifndef BONE_ANIMATION_TRAFOS_BINDING_POINT
#define BONE_ANIMATION_TRAFOS_BINDING_POINT 1
#endif

#endif

layout (location = 0) in vec3  position;
layout (location = 1) in vec3  normal;
layout (location = 2) in vec2  texCoords;
layout (location = 3) in vec3  tangent;
#if BONE_ANIMATION
layout (location = 4) in uvec4 boneId;
layout (location = 5) in vec4  boneWeight;
#endif

#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#include "interface/buffers.h"


#if BONE_ANIMATION
layout(column_major, std140, binding = BONE_ANIMATION_TRAFOS_BINDING_POINT) buffer BoneAnimationBuffer {
    mat4[] trafos;
} boneTrafos;

#endif



out VS_OUT {
	vec4 fragment_position_eye;
	vec4 fragment_position_world;
	vec4 camera_position_world;
	
    //float viewSpaceZ;
    vec4 position_ndc;
    vec4 position_ndc_previous;
	vec2 tex_coords;
	mat3 TBN_eye_directions; // used to transform the normal vector from tangent to eye space. 
						  //  This matrix mustn't be used with positions!!!
} vs_out;	

void commonVertexShader() {
    
#if BONE_ANIMATION
    mat4 boneTrafo = boneTrafos.trafos[boneId[0]] * boneWeight[0];
    boneTrafo += boneTrafos.trafos[boneId[1]] * boneWeight[1];
    boneTrafo += boneTrafos.trafos[boneId[2]] * boneWeight[2];
    boneTrafo += boneTrafos.trafos[boneId[3]] * boneWeight[3];
    
    /*boneTrafo = mat4(1.0) * boneWeight.x;
    boneTrafo += mat4(1.0) * boneWeight.y;
    boneTrafo += mat4(1.0) * boneWeight.z;
    boneTrafo += mat4(1.0) * boneWeight.w;*/
    
	
	/*mat4 defaultScale = mat4(0.03);
	defaultScale[3][3] = 1.0;
	mat4 invDefaultScale = mat4(1.0/ 0.03);
	invDefaultScale[3][3] = 1.0;
	
	boneTrafo = defaultScale * boneTrafo * invDefaultScale;*/
	
	
    vec4 positionLocal = boneTrafo * vec4(position, 1.0f);
	vec3 normalLocal = vec3(boneTrafo * vec4(normal, 0.0));
	vec3 tangentLocal = vec3(boneTrafo * vec4(tangent, 0.0));
	
	//Note: we have transform normal and tangent. We know that the bone trafo has no shearing, so it is ok not to inverse transpose it!
	
	
	//positionLocal.xyz *= 0.03;  1.0 / 0.03 *
#else 
    vec4 positionLocal = vec4(position, 1.0f);
	vec3 normalLocal = normal;
	vec3 tangentLocal = tangent;
#endif
    
    
    //constants.projectionGPass * constants.viewGPass * objectData.model 
    gl_Position =  objectData.transform * positionLocal;
	

	
    vs_out.position_ndc = objectData.transform * vec4(positionLocal.xyz, 1.0);
    vs_out.position_ndc_previous = objectData.prevTransform * vec4(positionLocal.xyz, 1.0);
    
    vs_out.tex_coords = texCoords;
    
    vs_out.fragment_position_eye = objectData.modelView * positionLocal;
	vs_out.fragment_position_world = objectData.model * positionLocal;
	vs_out.camera_position_world =  constants.invViewGPass * vec4(0,0,0,1);
	
	mat3 normalMatrix = objectData.normalMatrix;
	
	vec3 normal_eye = normalize(normalMatrix * normalLocal);
	vec3 tangent_eye = normalize(normalMatrix * tangentLocal);
	tangent_eye = normalize(tangent_eye - (dot(normal_eye, tangent_eye) * normal_eye));
	
	vec3 bitangent_eye = cross(normal_eye, tangent_eye); //normalize(objectData.normalMatrix * bitangent);
	
	float dotTN = dot(normal_eye, tangent_eye);
	
	// TBN must form a right handed coord system.
    // Some models have symetric UVs. Check and fix.
    if (dot(cross(normal_eye, tangent_eye), bitangent_eye) < 0.0)
        tangent_eye = tangent_eye * -1.0;

	vs_out.TBN_eye_directions = mat3(tangent_eye, bitangent_eye, normal_eye);
}