#version 330 core

uniform sampler2D image;

uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
uniform float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );
								   
uniform float windowHeight = 1024.0;
uniform float windowWidth = 1024.0;

out vec4 FragmentColor;

void main(void)
{	
    vec2 coord = vec2(gl_FragCoord.x / windowWidth, gl_FragCoord.y / windowHeight);	
    FragmentColor = texture2D( image, coord) * weight[0];
	
    for (int i=1; i<3; i++) {
		vec2 currOffset = vec2(offset[i], 0.0)/windowWidth;
        FragmentColor +=
            texture2D( image, coord + currOffset) * weight[i];
        FragmentColor +=
            texture2D( image, coord - currOffset) * weight[i];
    }
}