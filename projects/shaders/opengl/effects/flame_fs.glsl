#version 430

#include "effects/gradient_noise.glsl"
#include "effects/tiling_and_offset.glsl"

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

void main()
{    		    
    float distortionAmount = 0.1;
    vec2 timedOffset = time * vec2(0.0, -1.0);
    vec3 hdrColor = 3.4 * vec3(1.498039, 0.3843137, 0.2117647);
    vec2 distortedUV;
    float noise;
    tilingAndOffset(fs_in.tex_coords, vec2(1,1), timedOffset, distortedUV);
    gradientNoise(distortedUV, 10.0, noise);
    
    distortedUV = vec2(noise);

    vec2 uv = mix(fs_in.tex_coords, distortedUV, distortionAmount);
    

    // albedo color
	float alpha = texture(structureTex, uv).a;
	vec4 color =  vec4(hdrColor * alpha, alpha);
    
    color.rgb = pow(color.rgb, vec3(1.0));
    
    FragColor = color;
    //LuminanceColor = vec4(luminanceOut, FragColor.a);
    LuminanceColor = vec4(color.rgb, alpha);
}