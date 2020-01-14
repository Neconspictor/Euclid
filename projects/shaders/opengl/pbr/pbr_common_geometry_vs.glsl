#ifndef PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT
#define PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT 0
#endif

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

layout(column_major, std140, binding = PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT) buffer TransformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 transform;
    mat4 prevTransform;
    mat4 modelView;
    mat3 normalMatrix;
} transforms;


#if BONE_ANIMATION
layout(column_major, std140, binding = BONE_ANIMATION_TRAFOS_BINDING_POINT) buffer BoneAnimationBuffer {
    mat4[] trafos;
} boneTrafos;

#endif



out VS_OUT {
	vec4 fragment_position_eye;
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
	//positionLocal.xyz *= 0.03;  1.0 / 0.03 *
#else 
    vec4 positionLocal = vec4(position, 1.0f);
#endif
    
    
    
    gl_Position = transforms.transform * positionLocal;
	
    vs_out.position_ndc = gl_Position;
    vs_out.position_ndc_previous = transforms.prevTransform * positionLocal;
    
    vs_out.tex_coords = texCoords;
    
    vs_out.fragment_position_eye = transforms.modelView * positionLocal;
	
	vec3 normal_eye = normalize(transforms.normalMatrix * normal);
	vec3 tangent_eye = normalize(transforms.normalMatrix * tangent);
	tangent_eye = normalize(tangent_eye - (dot(normal_eye, tangent_eye) * normal_eye));
	
	vec3 bitangent_eye = cross(normal_eye, tangent_eye); //normalize(transforms.normalMatrix * bitangent);
	
	float dotTN = dot(normal_eye, tangent_eye);
	
	// TBN must form a right handed coord system.
    // Some models have symetric UVs. Check and fix.
    //if (dot(cross(normal_eye, tangent_eye), bitangent_eye) < 0.0)
    //    tangent_eye = tangent_eye * -1.0;

	vs_out.TBN_eye_directions = mat3(tangent_eye, bitangent_eye, normal_eye);
}