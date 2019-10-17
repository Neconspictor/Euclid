/**
 * Calculates the minimum and maximum z-value of the scene (seen from the camera)  
 */
#version 430 core

#define GROUP_NUM_X 1
#define GROUP_NUM_Y 1

#define PI      3.14159265358979323846
#define TWO_PI  6.28318530717958647692
#define GRAVITY 9.81

struct Complex {
    float re;
    float im;
};

layout (local_size_x = GROUP_NUM_X, local_size_y = GROUP_NUM_Y) in;

// read-write
layout (rg32f, binding = 0) uniform image2D resultHeight;

// writeonly
layout (rg32f, binding = 1) uniform image2D resultSlopeX;
layout (rg32f, binding = 2) uniform image2D resultSlopeZ;
layout (rg32f, binding = 3) uniform image2D resultDx;
layout (rg32f, binding = 4) uniform image2D resultDz;

// readonly
layout (rgba32f, binding = 5) uniform image2D heightZero;

uniform uvec2 uniquePointCount;
uniform vec2 waveLength;
uniform float currentTime;
uniform float periodTime;



Complex mul(in Complex c, float scalar) {
    
    Complex r;
    r.re = scalar * c.re;
    r.im = scalar * c.im;
    
    return r;
}

Complex mul(in Complex a, in Complex b) {
    
    Complex r;
    r.re = a.re * b.re - a.im * b.im;
    r.im = a.re * b.im + a.im * b.re;
    
    return r;
}

Complex add(in Complex a, in Complex b) {
    
    Complex r = Complex(a.re + b.re, a.im + b.im);
    return r;
}

void main(void)
{
    const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    
    const vec2 wave = (TWO_PI * index - (PI * vec2(uniquePointCount))) / waveLength;
    
    const float w0 = TWO_PI / periodTime;
    const float dispersion = floor(sqrt(GRAVITY * length(wave)) / w0) * w0;
    const float timedDispersion = dispersion * currentTime;
    const float real = cos(timedDispersion);
    const float imag = sin(timedDispersion);
    
    const vec4 heightZeroPack = imageLoad(heightZero, index);
    
    const Complex heightZero = Complex(heightZeroPack.x, heightZeroPack.y);
    const Complex heightZeroNegativeWaveConjugate = Complex(heightZeroPack.z, heightZeroPack.w);
    
    const Complex complexHeight = add(mul(heightZero, Complex(real, imag)), 
                                mul(heightZeroNegativeWaveConjugate, Complex(-real, -imag)));
                                
    const Complex complexSlopeX = mul(complexHeight, Complex(0, wave.x));
    const Complex complexSlopeZ = mul(complexHeight, Complex(0, wave.y));

    Complex complexDx = Complex(0.0, 0.0);
    Complex complexDz = Complex(0.0, 0.0);
    
    const float len = length(wave);
    
    if (len > 0.000001) {
        complexDx = mul(complexHeight, Complex(0, -wave.x / len));
        complexDz = mul(complexHeight, Complex(0, -wave.y / len));
    }
    
    
    
    
    
    const vec2 height = vec2(complexHeight.re, complexHeight.im);//vec2(currentTime, periodTime);
    const vec2 slopeX = vec2(complexSlopeX.re, complexSlopeX.im);
    const vec2 slopeZ = vec2(complexSlopeZ.re, complexSlopeZ.im);
    const vec2 dx     = vec2(complexDx.re, complexDx.im);
    const vec2 dz     = vec2(complexDz.re, complexDz.im);

    imageStore(resultHeight, index, vec4(height, 0.0, 0.0));
    imageStore(resultSlopeX, index, vec4(slopeX, 0.0, 0.0));
    imageStore(resultSlopeZ, index, vec4(slopeZ, 0.0, 0.0));
    imageStore(resultDx, index, vec4(dx, 0.0, 0.0));
    imageStore(resultDz, index, vec4(dz, 0.0, 0.0));
}