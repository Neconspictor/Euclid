#version 330

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

layout(binding = 0) uniform sampler2D sourceTexture;

void main() {
    fragColor = texture(sourceTexture, fs_in.texCoord);
}