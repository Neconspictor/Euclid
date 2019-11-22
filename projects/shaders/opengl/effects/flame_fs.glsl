#version 430

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

uniform sampler2D structureTex;
uniform vec4 baseColor;

#include "effects/gradient_noise.glsl"

void main()
{    		    
    // albedo color
	float alpha = texture(structureTex, fs_in.tex_coords).a;
	vec4 color =  vec4(20.3, 7.0, 2, alpha);
    
    FragColor = vec4(0.0);
    //LuminanceColor = vec4(luminanceOut, FragColor.a);
    LuminanceColor = vec4(alpha * color.rgb, alpha);
}