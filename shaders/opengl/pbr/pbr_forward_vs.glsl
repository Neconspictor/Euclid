#version 430 core
#include "pbr/pbr_common_geometry_vs.glsl"

out VS_OUT2 {
    vec4 fragment_position_eye;
} vs_out2;

void main() {
    commonVertexShader();
    vs_out2.fragment_position_eye = transforms.modelView * vec4(position, 1.0f);
}