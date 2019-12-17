#version 460 core
#include "pbr/pbr_common_geometry_vs.glsl"

void main() {
    //commonVertexShader();
    
    #if BONE_ANIMATION
    mat4 boneTrafo = boneTrafos.trafos[boneId[0]] * boneWeight[0];
    boneTrafo += boneTrafos.trafos[boneId[1]] * boneWeight[1];
    boneTrafo += boneTrafos.trafos[boneId[2]] * boneWeight[2];
    boneTrafo += boneTrafos.trafos[boneId[3]] * boneWeight[3];
    
    /*boneTrafo = mat4(1.0) * boneWeight.x;
    boneTrafo += mat4(1.0) * boneWeight.y;
    boneTrafo += mat4(1.0) * boneWeight.z;
    boneTrafo += mat4(1.0) * boneWeight.w;*/
    
    vec4 positionLocal = boneTrafo * vec4(position, 1.0f);
#else 
    vec4 positionLocal = vec4(position, 1.0f);
#endif
    
    
    mat4 view = mat4(inverse(mat3(transforms.view)));
    
    vec4 cameraRightWS = vec4(view[0][0], view[0][1], view[0][2], 0.0);
    vec4 cameraUpWS = vec4(view[1][0], view[1][1], view[1][2], 0.0);
    vec4 cameraLookWS = vec4(view[2][0], view[2][1], view[2][2], 0.0);
    
    vec4 positionWS = vec4(vec3(transforms.model[3]), 1.0);// + view * positionLocal;
    positionWS += vec4(0.0, positionLocal.y, 0.0, 0.0);
    positionWS += view[0] * positionLocal.x;
    //positionWS += view[1] * positionLocal.y;
    //positionWS += view[2] * positionLocal.z;
    //positionWS += view[3] * positionLocal.w;
    //positionWS += cameraRightWS * positionLocal.x + cameraUpWS * positionLocal.y + cameraLookWS * positionLocal.z;
    
    gl_Position = transforms.projection * transforms.view * positionWS;    
    vs_out.tex_coords = texCoords;
    
}