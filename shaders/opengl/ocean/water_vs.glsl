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
} vs_out;


layout(binding = 0) uniform sampler2D height;
layout(binding = 1) uniform sampler2D slopeX;
layout(binding = 2) uniform sampler2D slopeZ;
layout(binding = 3) uniform sampler2D dX;
layout(binding = 4) uniform sampler2D dZ;

uniform mat4 transform;
uniform mat4 modelViewMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform vec2 windDirection;
uniform float animationTime;
uniform float tileSize;



vec4 clipToScreenPos(in vec4 pos) 
{
    vec4 o = pos * 0.5f;
    o.xy += o.w;
    o.zw = pos.zw;
    return o;
}


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
    
    mPosition += vec4(tileSize * (gl_InstanceID % 8),0, tileSize * (gl_InstanceID / 8), 0);                     

    vec3 mNormal = normalize(vec3(-mSlopeX.x, 1.0f, -mSlopeZ.x));
    
    
    //vs_out.normal = normalize(normalMatrix * normal);
    vs_out.normal = normalize(normalMatrix * mNormal);
    vs_out.normalWorld = normalize(mat3(inverse(transpose(modelMatrix))) * mNormal); // Note that water isn't rotated normally!
    vs_out.positionView = vec3(modelViewMatrix * mPosition);
    vs_out.waterSurfaceZeroView = vec3(mPosition.x, 0 , mPosition.z);
    vs_out.waterSurfaceZeroView = vec3(modelViewMatrix * vec4(vs_out.waterSurfaceZeroView, 1));
    vs_out.positionWorld = vec3(modelMatrix * mPosition);
    vs_out.texCoords = texCoords;

    vec4 positionCS = transform * mPosition;

    vs_out.positionCS = clipToScreenPos(positionCS);


    gl_Position = positionCS;
}