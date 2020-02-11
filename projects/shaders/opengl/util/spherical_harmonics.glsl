#ifndef SPHERICAL_HARMONICS_H
#define SPHERICAL_HARMONICS_H

/**
 * Coeffeicients can be stored in a Texture1D or a Texture1DArray.
 */ 
#ifdef SAMPLER_1D_SH
	#ifndef SAMPLER_TYPE_SH
		#define SAMPLER_TYPE_SH sampler1D
	#endif
#else 
	#ifndef SAMPLER_TYPE_SH
		#define SAMPLER_TYPE_SH sampler1DArray
	#endif
#endif


#ifndef NO_PI_SH
const float PI = 3.14159265359;
#endif


float getCosineLobeFactorSH(int index) {
    if(index==0) {
        return PI;
    }
    else if(index==1){
        return 2.0/3.0 * PI;
    }
    else if(index==2){
        return 2.0/3.0 * PI;
    }
    else if(index==3){
        return 2.0/3.0 * PI;
    }
    else if(index==4){
        return PI/ 4.0;
    }
    else if(index==5){
        return PI/ 4.0;
    }
    else if(index==6){
        return PI/ 4.0;
    }
    else if(index==7){
        return PI/ 4.0;
    }
    else {
        return PI/ 4.0;
    }
}

float harmonicsSH(int index, in vec3 direction) {
    
    if(index==0) {
        return 0.282095;
    }
    else if(index==1){
        return 0.488603 * direction.y;
    }
    else if(index==2){
        return 0.488603 * direction.z;
    }
    else if(index==3){
        return 0.488603 * direction.x;
    }
    else if(index==4){
        return 1.092548 * direction.x * direction.y;
    }
    else if(index==5){
        return 1.092548 * direction.y * direction.z;
    }
    else if(index==6){
        return 0.315392 * (3.0 * direction.z * direction.z - 1.0);
    }
    else if(index==7){
        return 1.092548 * direction.x * direction.z;
    }
    else {
        return 0.546274 * (direction.x * direction.x - direction.y * direction.y);
    }
}


#ifdef SAMPLER_1D_SH
vec3 computeIrradiance(in sampler1D coefficients, in vec3 direction) {
#else 
vec3 computeIrradiance(in sampler1DArray coefficients, const in int arrayIndex, in vec3 direction) {
#endif

	vec3 irradiance = vec3(0.0);
    for (int i = 0; i < 9; ++i) {
        const float factor = getCosineLobeFactorSH(i);
        const float Ylm = harmonicsSH(i, direction);
        #ifdef SAMPLER_1D_SH
			const vec3 Llm = texelFetch(coefficients, i, 0).rgb;
		#else
			const vec3 Llm = texelFetch(coefficients, ivec2(i, arrayIndex), 0).rgb;
		#endif
        irradiance += factor * Llm * Ylm;
    }
	
	return irradiance;
}

#endif


#ifdef BUFFERS_DEFINE_MATERIAL_BUFFER

vec3 computeIrradiance(in PerObjectMaterialData materialData, in vec3 direction) {

	vec3 irradiance = vec3(0.0);
    for (int i = 0; i < 9; ++i) {
        const float factor = getCosineLobeFactorSH(i);
        const float Ylm = harmonicsSH(i, direction);
		const vec3 Llm = materialData.diffuseSHCoefficients[i].xyz;
        irradiance += factor * Llm * Ylm;
    }
	
	return irradiance;
}

#endif
