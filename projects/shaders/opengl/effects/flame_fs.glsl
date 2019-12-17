#version 460 core

#include "effects/gradient_noise.glsl"
#include "effects/tiling_and_offset.glsl"
#include "effects/voronoi.glsl"

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;

in VS_OUT {	
	vec4 fragment_position_eye;
    //float viewSpaceZ;
    vec4 position_ndc;
    vec4 position_ndc_previous;
	vec2 tex_coords;
	mat3 TBN_eye_directions; // used to transform the normal vector from tangent to eye space.
						  //  This matrix mustn't be used with positions!!!
} fs_in;

layout(binding = 0) uniform sampler2D structureTex;
uniform vec4 baseColor;
uniform float time;


const float dissolveAmount = 0.2;
const vec2 dissolveSpeed = vec2(-0.005, -0.04);
const float dissolveScale = 20.0;


const float distortionAmount = 0.5;
const vec2 distortionSpeed = vec2(0.0, -0.2);
const float distortionScale = 10.0;

const vec4 hdrColor = vec4(2.0 * vec3(7.906699, 2.069816, 1.117701), 1.0);

void createFireStructure(float noise, in vec4 structureColor, out vec4 fireStructureColor) 
{
    vec2 uv;
    float voronoiValue;
    float voronoiCells;
    vec2 timedOffset = time * dissolveSpeed;
    tilingAndOffset(fs_in.tex_coords, vec2(1,1), timedOffset, uv);
    voronoi(uv, 2, dissolveScale, voronoiValue, voronoiCells);
    float dissolveAmount = noise * pow(voronoiValue, dissolveAmount);
    
    fireStructureColor = dissolveAmount * structureColor * (1 - fs_in.tex_coords.y);
}


void main()
{    		    
    
    vec2 timedOffset = time * distortionSpeed;
    vec2 distortedUV;
    float noise;
    tilingAndOffset(fs_in.tex_coords, vec2(1,1), timedOffset, distortedUV);
    
    gradientNoise(distortedUV, distortionScale, noise);
    
    distortedUV = vec2(noise);

    vec2 uv = mix(fs_in.tex_coords, distortedUV, 0.5 * fs_in.tex_coords.y * distortionAmount);
    

    
    vec4 structureColor = texture(structureTex, uv);
    vec4 fireStructureColor;
    createFireStructure(noise, structureColor, fireStructureColor);
    
    
	vec4 color =  hdrColor * fireStructureColor;
    color.a = fireStructureColor.a;
    
    FragColor = color; //texture(structureTex, fs_in.tex_coords);
    //LuminanceColor = vec4(luminanceOut, FragColor.a);
    LuminanceColor =  FragColor;//vec4(pow(color.a, 2.0) * pow(FragColor.rgb, vec3(1.0)), color.a);
}