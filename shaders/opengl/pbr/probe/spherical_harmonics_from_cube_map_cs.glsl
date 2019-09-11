#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0) uniform samplerCube environmentMaps;
layout (rgba32f, binding = 1) uniform image2D result;

uniform uint rowStart;

#define PI      3.14159265358979323846
#define TWO_PI  6.28318530717958647692


int lookupL [9] = {
    0,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    2
};

int lookupM [9] = {
    0,
    -1,
    0,
    1,
    -2,
    -1,
    0,
    1,
    2
};

float harmonics(int index, in vec3 direction) {
    
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
        return 0.315392 * (3.0* direction.z * direction.z - 1.0);
    }
    else if(index==7){
        return 1.092548 * direction.x * direction.z;
    }
    else {
        return 0.546274 * (direction.x * direction.x - direction.y * direction.y);
    }
}

float getScale(int index) {
    
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

void main()
{		
    const int harmonicIndex = int(gl_WorkGroupID.x);
    //const int l = lookupL(id);
    //const int m = lookupM(id);
    const int mapIndex = int(rowStart + gl_WorkGroupID.y);
	const float sampleDelta = 0.025; // 0.025
	//const int nrSamples = int (TWO_PI / sampleDelta) * int(PI / sampleDelta); 
    
    vec3 irradiance = vec3(0.0);
    float nrSamples = 0.0;
    
	for(float theta = 0.0; theta < PI; theta += sampleDelta)
	{
		for(float phi = 0.0; phi < TWO_PI; phi += sampleDelta)
		{
			// spherical to cartesian (in world space)
			vec3 sampleVec = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            sampleVec = normalize(sampleVec);
            const float Ylm = harmonics(harmonicIndex, sampleVec);
            //vec4(sampleVec, mapIndex)
			irradiance += texture(environmentMaps, sampleVec).rgb * Ylm * sin(theta);//* sin(theta);
            ++nrSamples;
		}
	}
	irradiance = 2 * PI * getScale(harmonicIndex) * irradiance * (1.0 / float(nrSamples)); //2 * PI * PI *              getScale(harmonicIndex) *
   
    //imageStore(result, ivec2(harmonicIndex, mapIndex), vec4(float(harmonicIndex), float(mapIndex), 0.0, 1.0));
    imageStore(result, ivec2(harmonicIndex, mapIndex), vec4(irradiance, 1.0));
    //imageStore(result, ivec2(harmonicIndex, mapIndex), vec4(1.0, 1.0, 1.0, 1.0));
}