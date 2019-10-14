#version 420

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

layout(binding = 0) uniform sampler2D aoMap;


void main() {
    
    float ao = texture(aoMap, fs_in.texCoord).r;
    
    // we use multiplicative blending
    fragColor = vec4(ao, ao, ao, 1.0);
}