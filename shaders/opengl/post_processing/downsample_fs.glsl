#version 330

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

layout(binding = 0) uniform sampler2D sourceTexture;

void main() {


    vec2 texelSize = 1.0 / textureSize(sourceTexture, 0).xy;
   
   /* float scale = 0.5;

    fragColor =  texture(sourceTexture, fs_in.texCoord + scale * texelSize * vec2(-1,-1));
    fragColor += texture(sourceTexture, fs_in.texCoord + scale * texelSize * vec2(1, -1));
    fragColor += texture(sourceTexture, fs_in.texCoord + scale * texelSize * vec2(-1, 1));
    fragColor += texture(sourceTexture, fs_in.texCoord + scale * texelSize * vec2(1,  1));
    fragColor /= 4.0;*/
    
    
    /*
       Lanzcos resampling:
        sinc(x) * sinc(x/a) = (a * sin(pi * x) * sin(pi * x / a)) / (pi^2 * x^2)
        Assuming a Lanczos constant of 2.0, and scaling values to max out at x = +/- 1.5    
        https://stackoverflow.com/questions/14366672/how-can-i-improve-this-webgl-glsl-image-downsampling-shader
        
         Steps: 3/8, 6/8, 9/8, 12/8
        1 + 2*0.7396427920000761
        + 2*0.2353466775192183
        + 2*-0.060095064454162
        + 2*-0.0636843520279026 = 2.7024201060744596
        result is normalized by 2.7024201060744596 so that sum of luminance remains 1.0
    
    */
    vec2 firstOffset = texelSize;
    vec2 secondOffset  = 2.0*texelSize;
    vec2 thirdOffset  = 3.0*texelSize;
    vec2 fourthOffset  = 4.0*texelSize;
    
    vec2 centerTextureCoordinate = fs_in.texCoord;
    vec2 oneStepLeftTextureCoordinate = fs_in.texCoord - firstOffset;
    vec2 twoStepsLeftTextureCoordinate = fs_in.texCoord - secondOffset;
    vec2 threeStepsLeftTextureCoordinate = fs_in.texCoord - thirdOffset;
    vec2 fourStepsLeftTextureCoordinate = fs_in.texCoord - fourthOffset;
    vec2 oneStepRightTextureCoordinate = fs_in.texCoord + firstOffset;
    vec2 twoStepsRightTextureCoordinate = fs_in.texCoord + secondOffset;
    vec2 threeStepsRightTextureCoordinate = fs_in.texCoord + thirdOffset;
    vec2 fourStepsRightTextureCoordinate = fs_in.texCoord + fourthOffset;
    
    fragColor = texture(sourceTexture, centerTextureCoordinate) * 0.38026; // 1.0 / 2.7024201060744596

    fragColor += texture(sourceTexture, oneStepLeftTextureCoordinate) * 0.27667; //sinc(3/8) * sinc((3/8.0)/2.0) / 2.7024201060744596
    fragColor += texture(sourceTexture, oneStepRightTextureCoordinate) * 0.27667; //sinc(3/8) * sinc((3/8.0)/2.0) / 2.7024201060744596

    fragColor += texture(sourceTexture, twoStepsLeftTextureCoordinate) * 0.08074; //sinc(6/8) * sinc((6/8.0)/2.0) / 2.7024201060744596
    fragColor += texture(sourceTexture, twoStepsRightTextureCoordinate) * 0.08074; //sinc(6/8) * sinc((6/8.0)/2.0) / 2.7024201060744596

    fragColor += texture(sourceTexture, threeStepsLeftTextureCoordinate) * -0.02612; //sinc(9/8) * sinc((9/8.0)/2.0) / 2.7024201060744596
    fragColor += texture(sourceTexture, threeStepsRightTextureCoordinate) * -0.02612; //sinc(9/8) * sinc((9/8.0)/2.0) / 2.7024201060744596

    fragColor += texture(sourceTexture, fourStepsLeftTextureCoordinate) * -0.02143; //sinc(12/8) * sinc((12/8.0)/2.0) / 2.7024201060744596
    fragColor += texture(sourceTexture, fourStepsRightTextureCoordinate) * -0.02143; //sinc(12/8) * sinc((12/8.0)/2.0) / 2.7024201060744596
}