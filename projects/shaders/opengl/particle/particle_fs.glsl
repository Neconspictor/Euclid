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

    uint tileIndex = uint((1.0 - lifeTimePercentage) * (tileCount.x * tileCount.y - 1));
    vec2 uv = texCoordsFS;
    toAtlasUvSpace(uv, tileCount, tileIndex);

    color = baseColor * texture(tex, uv);
    luminance = vec4(0.0, 0.0, 0.0, 1.0);
}