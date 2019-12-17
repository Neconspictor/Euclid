#version 430 core

#include "util/texture_atlas.glsl"

struct Particle {
    mat4 worldTrafo;
    float lifeTimePercentage;
    //float _pad[3]; <- for host application
};


layout (location = 0) in vec2 position;


uniform mat4 viewProj;
uniform mat3 invView3x3;
//uniform mat4 model;

//uniform float lifeTimePercentage;
uniform uvec2 tileCount;

out vec2 uv;
out vec2 uvNext;
//out int instanceID;
out float lifeTimePercentage;

layout(std430, binding = 0) buffer ParticlesBuffer {
    Particle particles[];
};

void main()
{    
    vec4 positionLocal = vec4(position, 0.0, 1.0);    
    //vec4 positionWS = vec4(invView3x3 * positionLocal.xyz, 1.0);//positionLocal + vec4(vec3(model[3]), 0.0);
    //positionWS.xyz += vec3(model[3]);
    //positionWS += vec4(0.0, position.y, 0.0, 0.0);
    //positionWS += vec4(invView3x3[0] * position.x, 0.0);

    //instanceID = gl_InstanceID;
    lifeTimePercentage = particles[gl_InstanceID].lifeTimePercentage;

    gl_Position = viewProj * particles[gl_InstanceID].worldTrafo * positionLocal; 
    
    uint tileIndexMax = tileCount.x * tileCount.y - 1;
    float tileIndexFloat = lifeTimePercentage * tileIndexMax;
    uint tileIndex = uint(tileIndexFloat);
    uint tileIndexNext = min(tileIndex + 1, tileIndexMax);
    
    
    uv = position + 0.5;
    uvNext = uv;
    
    toAtlasUvSpace(uv, tileCount, tileIndex);
    toAtlasUvSpace(uvNext, tileCount, tileIndexNext);
    
} 