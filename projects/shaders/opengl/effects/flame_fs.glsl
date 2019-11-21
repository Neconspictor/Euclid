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

void main()
{    		    
    // albedo color
	vec4 color = texture(structureTex, fs_in.tex_coords);
	color *= baseColor;
    
    FragColor = color;
    //LuminanceColor = vec4(luminanceOut, FragColor.a);
    LuminanceColor = vec4(max(color.rgb - vec3(1.0), vec3(0.0)), color.a);
}