#version 330 core

uniform sampler2D image;

uniform float offset[5] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
uniform float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216,
                                   0.0540540541, 0.0162162162 );
								   
uniform float windowWidth = 1.0;

out vec4 FragmentColor;

void main(void)
{
	vec2 size = textureSize(image, 0);
	float width = size.x;
	float height = size.y;
    vec2 coord = vec2(gl_FragCoord.x / width, gl_FragCoord.y / height);
	
    FragmentColor = texture2D( image, coord) * weight[0];
    for (int i=1; i<5; i++) {
		vec2 currOffset = vec2(offset[i], 0.0)/width;
        FragmentColor +=
            texture2D( image, coord + currOffset) * weight[i];
        FragmentColor +=
            texture2D( image, coord - currOffset) * weight[i];
    }
}