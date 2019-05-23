#version 450
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;
/*layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;*/


out VS_OUT {
    vec3 normal;
    vec3 positionView;
    //vec2 texCoords;
} vs_out;


layout(binding = 0) uniform sampler2D height;
layout(binding = 1) uniform sampler2D slopeX;
layout(binding = 2) uniform sampler2D slopeZ;
layout(binding = 3) uniform sampler2D dX;
layout(binding = 4) uniform sampler2D dZ;

uniform mat4 transform;
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;


void main() { 
    
   vec2 mSlopeX =  texture(slopeX, texCoords).xy;
   vec2 mSlopeZ =  texture(slopeZ, texCoords).xy;
   vec2 mDX =  texture(dX, texCoords).xy;
   vec2 mDZ =  texture(dZ, texCoords).xy;
   float mLambda = -1.0;
   
   
   vec4 mPosition = vec4(position.x + mLambda * mDX.x,
                         texture(height, texCoords).x,
                         position.z + mLambda * mDZ.x,
                         1.0);
   
   vec3 mNormal = normalize(vec3(-mSlopeX.x, 1.0f, -mSlopeZ.x));
    
    
  //vs_out.normal = normalize(normalMatrix * normal);
  vs_out.normal = normalize(normalMatrix * mNormal);
  vs_out.positionView = vec3(modelViewMatrix * mPosition);
  //vs_out.texCoords = texCoords;
  
  gl_Position = transform * mPosition;  
}