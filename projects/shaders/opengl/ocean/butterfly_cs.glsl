/**
 * Calculates the butterfly texture used for fft  
 */
#version 460 core

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

// writeonly
layout (rgba32f, binding = 0) uniform image2D butterfly;

uniform int N;


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


/**
 * Reverses the bits of an integer using 'bitCount' bits
 */
int bitrev(int n, int bitCount)
{
	// Find the max number for bit reversing. All bits higher than this number will be zeroed.
	// the max number is determined by 2^bitCount - 1.
	const int maxNumber = (1 << bitCount) - 1;
	// Initialize the reversed number 
	int reversedN = n;

	// Shift n to the right, reversed n to the left, 
	// and give least-significant bit of n to reversed n.
	// Do this process as long as n is greater zero.
	// Technically we have to do this process bitCount times
	// But we can save some loops by ignoring shifts if n is zero and 
	// just left shift reversed n by the remaining bits.
	// Therefore we need the remainingBits variable.
	int remainingBits = bitCount - 1;
	for (n >>= 1; n > 0; n >>= 1)
	{
		reversedN <<= 1;
		reversedN |= n & 1;
		remainingBits--;
	}

	// left shift reversed n by the remaining bits.
	reversedN <<= remainingBits;

	// Clear all bits more significant than the max number 
	reversedN &= maxNumber;

	return reversedN;
}

/**
 * Calculates twiddle factor W(k,N)= e^(i * 2*pi * k / N ) for the current fft recursion stage
 * (y-component of the global invocation) 
 */
/*void main(void)
{
    const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    
    const int fftStage = index.y;
    const int stageN = int(pow(2, fftStage + 1));
    const float stageNFloat = stageN;
    const int fftSampleIndex = index.x;
    const int k = fftSampleIndex % stageN;
    
    const float twiddleExponent = TWO_PI * k / stageNFloat;
    const Complex twiddle = Complex(cos(twiddleExponent), sin(twiddleExponent));
    
    int evenStageNHalfthSampleIndex;
    int oddStageNHalfthSampleIndex;
    
    if (k < (stageN / 2)) {
        
        // At the first fft stage we have to bit reverse the indices 
        if (fftStage == 0) {
            const int bitCount = int(log2(N));
            evenStageNHalfthSampleIndex = bitrev(fftSampleIndex, bitCount);
            oddStageNHalfthSampleIndex = bitrev(fftSampleIndex + 1, bitCount);
        } else {
            evenStageNHalfthSampleIndex = fftSampleIndex;
            oddStageNHalfthSampleIndex = fftSampleIndex +  (stageN / 2);
        }
    } else {
    
        // At the first fft stage we have to bit reverse the indices 
        if (fftStage == 0) {
            const int bitCount = int(log2(N));
            evenStageNHalfthSampleIndex = bitrev(fftSampleIndex - 1, bitCount);
            oddStageNHalfthSampleIndex = bitrev(fftSampleIndex, bitCount);
        } else {
            evenStageNHalfthSampleIndex = fftSampleIndex - (stageN / 2);
            oddStageNHalfthSampleIndex = fftSampleIndex;
        }
    }
    

    imageStore(butterfly, index, vec4(twiddle.re, twiddle.im, evenStageNHalfthSampleIndex, oddStageNHalfthSampleIndex));
}*/

void main(void)
{
	vec2 x = gl_GlobalInvocationID.xy;
	float k = mod(x.x * (float(N)/ pow(2,x.y+1)), N);
	Complex twiddle = Complex( cos(TWO_PI*k/float(N)), sin(TWO_PI*k/float(N)));
	
	int butterflyspan = int(pow(2, x.y));
	
	int butterflywing;
	
	if (mod(x.x, pow(2, x.y + 1)) < pow(2, x.y))
		butterflywing = 1;
	else butterflywing = 0;

	// first stage, bit reversed indices
	if (x.y == 0) {
        const int bitCount = int(log2(N));
    
		// top butterfly wing
		if (butterflywing == 1)
			imageStore(butterfly, ivec2(x), vec4(twiddle.re, twiddle.im, bitrev(int(x.x), bitCount), bitrev(int(x.x + 1), bitCount)));
		// bot butterfly wing
		else	
			imageStore(butterfly, ivec2(x), vec4(twiddle.re, twiddle.im, bitrev(int(x.x - 1), bitCount), bitrev(int(x.x), bitCount)));
	}
	// second to log2(N) stage
	else {
		// top butterfly wing
		if (butterflywing == 1)
			imageStore(butterfly, ivec2(x), vec4(twiddle.re, twiddle.im, x.x, x.x + butterflyspan));
		// bot butterfly wing
		else
			imageStore(butterfly, ivec2(x), vec4(twiddle.re, twiddle.im, x.x - butterflyspan, x.x));
	}
}

/*void main(void)
{
	vec2 x = gl_GlobalInvocationID.xy;
	float k = mod(x.y * (float(N)/ pow(2,x.x+1)), N);
	Complex twiddle = Complex( cos(TWO_PI*k/float(N)), sin(TWO_PI*k/float(N)));
	
	int butterflyspan = int(pow(2, x.x));
	
	int butterflywing;
	
	if (mod(x.y, pow(2, x.x + 1)) < pow(2, x.x))
		butterflywing = 1;
	else butterflywing = 0;

	// first stage, bit reversed indices
	if (x.x == 0) {
        const int bitCount = int(log2(N));
    
		// top butterfly wing
		if (butterflywing == 1)
			imageStore(butterfly, ivec2(x), vec4(twiddle.re, twiddle.im, bitrev(int(x.y), bitCount), bitrev(int(x.y + 1), bitCount)));
		// bot butterfly wing
		else	
			imageStore(butterfly, ivec2(x), vec4(twiddle.re, twiddle.im, bitrev(int(x.y - 1), bitCount), bitrev(int(x.y), bitCount)));
	}
	// second to log2(N) stage
	else {
		// top butterfly wing
		if (butterflywing == 1)
			imageStore(butterfly, ivec2(x), vec4(twiddle.re, twiddle.im, x.y, x.y + butterflyspan));
		// bot butterfly wing
		else
			imageStore(butterfly, ivec2(x), vec4(twiddle.re, twiddle.im, x.y - butterflyspan, x.y));
	}
}*/