#version 460 core

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 color;

uniform sampler2D screenTexture;

const float offset = 1.0f / 300.0f;

void main()
{     
    // apply gamma correction
    float gamma = 2.2f;
	float value = texture(screenTexture, fs_in.texCoord).r;
    //col = pow(col.rgb, vec3(1.0 / gamma));
    color = vec4(value, value, value, 1);
}