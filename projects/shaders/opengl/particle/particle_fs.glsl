#version 330 core

in vec2 uv;
in vec2 uvNext;
in float lifeTimePercentage;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 luminance;

struct Particle {
    mat4 worldTrafo;
    float lifeTimePercentage;
    //float _pad[3]; <- for host application
};

layout(std430, binding = 0) buffer ParticlesBuffer {
    Particle particles[];
};


//uniform float lifeTimePercentage;
uniform uvec2 tileCount;

uniform vec4 baseColor;
layout(binding = 0) uniform sampler2D tex;

void main()
{
    uint tileIndexMax = tileCount.x * tileCount.y - 1;
    float tileIndexFloat = (lifeTimePercentage) * (tileIndexMax);
    float fractional = fract(tileIndexFloat);

    color = baseColor * mix(texture(tex, uv), texture(tex, uvNext), fractional);
    //color = baseColor * texture(tex, uv);
    luminance = vec4(0.0, 0.0, 0.0, 1.0);
}