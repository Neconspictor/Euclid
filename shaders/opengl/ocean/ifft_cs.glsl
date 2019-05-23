/**
 * Calculates the minimum and maximum z-value of the scene (seen from the camera)  
 */
#version 430 core

#define GROUP_NUM_X 1
#define GROUP_NUM_Y 1

#define PI      3.14159265358979323846
#define TWO_PI  6.28318530717958647692
#define GRAVITY 9.81

layout (local_size_x = GROUP_NUM_X, local_size_y = GROUP_NUM_Y) in;

// readonly
layout (rg32f, binding = 0) uniform image2D inputImage;
layout (rgba32f, binding = 1) uniform image2D butterfly;

// writeonly
layout (rg32f, binding = 2) uniform image2D outputImage;

uniform int N;
uniform int stage;

// specifies if ifft should operate horizontal (false) or vertically (true)
uniform int vertical;



struct Complex {
    float re;
    float im;
};

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

void horizontalOperation() {
    const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    
    const int k = index.x;
    
    const ivec2 butterflyIndex = ivec2(k, stage);
    
    const vec4 butterflyData = imageLoad(butterfly, butterflyIndex);
    
    Complex twiddle = Complex(butterflyData.x, butterflyData.y);
    
    const vec2 a = imageLoad(inputImage, ivec2(butterflyData.z, index.y)).xy;
    const vec2 b = imageLoad(inputImage, ivec2(butterflyData.w, index.y)).xy;
    
    Complex aComplex = Complex(a.x, a.y);
    Complex bComplex = Complex(b.x, b.y);
    
    Complex result = add(aComplex, mul(twiddle, bComplex));
    
    
    imageStore(outputImage, index, vec4(result.re, result.im, 0.0, 0.0));
}

void verticalOperation() {
    const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    
    const int k = int(gl_GlobalInvocationID.y);
    
    const ivec2 butterflyIndex = ivec2(k, stage);
    
    const vec4 butterflyData = imageLoad(butterfly, butterflyIndex);
    
    Complex twiddle = Complex(butterflyData.x, butterflyData.y);
    
    const vec2 a = imageLoad(inputImage, ivec2(index.x, butterflyData.z)).xy;
    const vec2 b = imageLoad(inputImage, ivec2(index.x, butterflyData.w)).xy;
    
    Complex aComplex = Complex(a.x, a.y);
    Complex bComplex = Complex(b.x, b.y);
    
    Complex result = add(aComplex, mul(twiddle, bComplex));
    
    
    imageStore(outputImage, index, vec4(result.re, result.im, 0.0, 0.0));
}

void main(void)
{
    if (vertical != 0) {
        verticalOperation();
    } else {
        horizontalOperation();
    } 
    //const vec2 wave = (TWO_PI * index - (PI * uniquePointCount)) / waveLength;
    //imageStore(outputImage, index, vec4(1.0, -1.0, 0.0, 0.0));
}