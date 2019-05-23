/**
 * Calculates the minimum and maximum z-value of the scene (seen from the camera)  
 */
#version 430 core

#define GROUP_NUM_X 1
#define GROUP_NUM_Y 1

#define PI      3.14159265358979323846
#define TWO_PI  6.28318530717958647692
#define INV_SQRT_TWO 0.7071067811865475  // 1 / sqrt(2)
#define GRAVITY 9.81

struct Complex {
    float re;
    float im;
};

layout (local_size_x = GROUP_NUM_X, local_size_y = GROUP_NUM_Y) in;

// An image to store data into.
layout (rgba32f, binding = 0) uniform image2D result;
layout (rgba32f, binding = 1) uniform image2D randTexture;

uniform uvec2 uniquePointCount;
uniform vec2 waveLength;
uniform float spectrumScale;
uniform vec2 windDirection;
uniform float windSpeed;


float getGaussianRand() {
    return 1.0f;
}


Complex mul(in Complex c, float scalar) {
    
    Complex r;
    r.re = scalar * c.re;
    r.im = scalar * c.im;
    
    return r;
}

float philipsSpectrum(vec2 k) {
    const float angle = dot(normalize(k), normalize(windDirection));
    const float absoluteAngle2 = angle * angle;
    const float kLength = length(k);
    const float kLength4 = kLength * kLength * kLength * kLength;
    const float L = windSpeed * windSpeed / GRAVITY;
    const float kWaveLength = kLength * L;
    const float kWaveLength2 = kWaveLength * kWaveLength;
    
    const float exponential = (exp(-1.0 / kWaveLength2) / kLength4);
    
    const float damping = 0.002;
    const float L2 = L * L;
	const float l2 = L2 * damping * damping;

    // exponential will be -inf if kLength is 0
    // we want to return 0 in this case.
    if (isnan(exponential)) return 0.0;
    return spectrumScale * exponential * absoluteAngle2 * exp(-kLength * kLength*l2);
}

void main(void)
{
    const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
 
    
    const vec2 wave = (TWO_PI * index - (PI * uniquePointCount)) / waveLength;
    
    const float heightZeroSpectrum = sqrt(philipsSpectrum(wave));
    const float heightZeroMinusWaveSpectrum = sqrt(philipsSpectrum(-wave));
    
    vec4 rand = imageLoad(randTexture, index);
    Complex heightZero;
    //heightZero.re = INV_SQRT_TWO * heightZeroSpectrum;
    //heightZero.im = INV_SQRT_TWO * heightZeroSpectrum;
    heightZero.re = INV_SQRT_TWO * rand.x * heightZeroSpectrum;
    heightZero.im = INV_SQRT_TWO * rand.y * heightZeroSpectrum;
    
    Complex heightZeroMinusWaveConjugate;
    heightZeroMinusWaveConjugate.re = INV_SQRT_TWO * rand.z * heightZeroMinusWaveSpectrum;
    heightZeroMinusWaveConjugate.im = -INV_SQRT_TWO * rand.w * heightZeroMinusWaveSpectrum;
    
    const vec4 data = vec4(
        heightZero.re, 
        heightZero.im, 
        heightZeroMinusWaveConjugate.re, 
        heightZeroMinusWaveConjugate.im
    );

    imageStore(result, index, data);
}