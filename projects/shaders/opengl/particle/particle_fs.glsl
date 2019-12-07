#version 330 core

#include "util/texture_atlas.glsl"

in vec2 texCoordsFS;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 luminance;

uniform vec4 baseColor;
uniform uvec2 tileCount;
uniform float lifeTimePercentage;

layout(binding = 0) uniform sampler2D tex;

void main()
{ 

    uint tileIndexMax = tileCount.x * tileCount.y - 1;
    float tileIndexFloat = (lifeTimePercentage) * (tileIndexMax);
    float fractional = fract(tileIndexFloat);
    
    uint tileIndex = uint(tileIndexFloat);
    uint tileIndexNext = min(tileIndex + 1, tileIndexMax);
    
    vec2 uv = texCoordsFS;
    vec2 uvNext = texCoordsFS;
    toAtlasUvSpace(uv, tileCount, tileIndex);
    toAtlasUvSpace(uvNext, tileCount, tileIndexNext);
    
    

    color = baseColor * mix(texture(tex, uv), texture(tex, uvNext), fractional);
    //color = baseColor * texture(tex, uv);
    luminance = vec4(0.0, 0.0, 0.0, 1.0);
}