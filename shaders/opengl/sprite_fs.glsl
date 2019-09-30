#version 330 core

in vec2 texCoordsFS;
out vec4 color;

uniform sampler2D sprite;

const float offset = 1.0f / 300.0f;

void main()
{ 
    // normal drawing
    // color = texture(screenTexture, texCoordsFS);
    // inverse color
    //color = vec4(vec3( 1.0 - texture(screenTexture, texCoordsFS)), 1.0f);
    
    // average grayscale
    //color = texture(screenTexture, texCoordsFS);
    //float average = (color.r + color.g + color.b) / 3.0;
    //color = vec4(average, average, average, 1.0f);

    // weighted grayscale
    //color = texture(screenTexture, texCoordsFS);
    //float weighted = 0.21 * color.r + 0.72 * color.g + 0.07* color.b;
    //color = vec4(weighted, weighted, weighted, 1.0f);
    
    //image kernels
    /*vec2 offsets[9] = vec2[] (
        vec2(-offset, offset),  // top-left
        vec2(0, offset),        // top-center
        vec2(offset, offset),   // top-right
        vec2(-offset, 0),       // center-left
        vec2(0,0),              // center-center
        vec2(offset, 0),        // center-right
        vec2(-offset, -offset), // bottom-left
        vec2(0, -offset),       // bottom-center
        vec2(offset, -offset)   // bottom-right
    );*/
    
    //sharpening kernel
    /*float kernel[9] = float[] (
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );*/
    
    // blur kernel
    /*float kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16  
    );*/
    
    // edge detection
    /*float kernel[9] = float[] (
        1,  1,  1,
        1, -8,  1,
        1,  1,  1
    );*/
    
    /*// normal drawing
    float kernel[9] = float[] (
        0,  0,  0,
        0, 1,  0,
        0,  0,  0
    );
	
    vec3 sampleTex[9];
    for(int i = 0; i < 9; ++i) {
        sampleTex[i] = vec3(texture(screenTexture, texCoordsFS.st + offsets[i]));
    }
    
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; ++i)
        col += sampleTex[i] * kernel[i];
		
	*/	
    color = texture(sprite, texCoordsFS);
}