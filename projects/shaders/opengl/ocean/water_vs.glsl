#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;
/*layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;*/


out VS_OUT {
    vec3 normal;
    vec3 normalWorld;
    vec3 positionView;
    vec3 waterSurfaceZeroView;
    vec3 positionWorld;
    vec4 positionCS;
    vec2 texCoords;
	vec2 slopeXZ;
} vs_out;


layout(binding = 0) uniform sampler2D height;
layout(binding = 1) uniform sampler2D slopeX;
layout(binding = 2) uniform sampler2D slopeZ;
layout(binding = 3) uniform sampler2D dX;
layout(binding = 4) uniform sampler2D dZ;

uniform vec2 windDirection;
uniform float animationTime;
uniform float tileSize;
uniform uvec2 tileCount;

#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#include "interface/buffers.h"
#include "util/depth_util.glsl"



vec4 getDisplacedPosition(in vec2 uv, float scale) {
   vec2 mDX =  texture(dX, uv).xy * scale;
   vec2 mDZ =  texture(dZ, uv).xy * scale;
   float mLambda = -1.0;
   
   
   vec4 result =  vec4(position.x + mLambda * mDX.x,
                         texture(height, uv).x * scale,
                         position.z + mLambda * mDZ.x,
                         1.0);
                         
    //result.x = position.x;
    //result.z = position.z;

    return result;
                         
}


void main() { 
    
    const float scale = 1.0; 
    vec2 offset = vec2(0.0);//animationTime * vec2(1, 1);
    vec2 uv = texCoords + offset; 
    
    vec4 mPosition = getDisplacedPosition(uv, scale);
    vec2 mSlopeX =  texture(slopeX, uv).xy * scale;
    vec2 mSlopeZ =  texture(slopeZ, uv).xy * scale;
	
    mPosition += vec4(tileSize * (gl_InstanceID % tileCount.x),
					0, 
					tileSize * (gl_InstanceID / tileCount.x), 
					0);                     

    vec3 mNormal = normalize(vec3(-mSlopeX.x, 1.0f, -mSlopeZ.x));
    
    
    //vs_out.normal = normalize(normalMatrix * normal);
    vs_out.normal = normalize(objectData.normalMatrix * mNormal);
    vs_out.normalWorld = normalize(vec3(constants.invViewGPass * vec4(mNormal, 0.0)));//normalize(mat3(inverse(transpose(objectData.model))) * mNormal); // Note that water isn't rotated normally!
    vs_out.positionView = vec3(objectData.modelView * mPosition);
    vs_out.waterSurfaceZeroView = vec3(mPosition.x, 0 , mPosition.z);
    vs_out.waterSurfaceZeroView = vec3(objectData.modelView * vec4(vs_out.waterSurfaceZeroView, 1));
    vs_out.positionWorld = vec3(objectData.model * mPosition);
    vs_out.texCoords = texCoords;
	vs_out.slopeXZ = vec2(mSlopeX.x, mSlopeZ.x);

    vec4 positionCS = objectData.transform * mPosition;

    vs_out.positionCS = clipToScreenSpace(positionCS);


    gl_Position = positionCS;
}