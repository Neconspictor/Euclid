#version 450 core

layout(triangles, equal_spacing, ccw) in;

in vec4 position_ndc_tes_in[];
in vec2 texCoord_ndc_tes_in[];

out vec4 position_ndc_fs_in;
out vec2 texCoord_ndc_fs_in;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2);
vec4 interpolate4D(vec4 v0, vec4 v1, vec4 v2);

void main() {
    
    // Interpolate the attributes of the output vertex using the barycentric coordinates
    position_ndc_fs_in = interpolate4D(position_ndc_tes_in[0], 
                                        position_ndc_tes_in[1],
                                        position_ndc_tes_in[2]);
                                        
    texCoord_ndc_fs_in =  interpolate2D(texCoord_ndc_tes_in[0], 
                                        texCoord_ndc_tes_in[1],
                                        texCoord_ndc_tes_in[2]);
                                        
    gl_Position = position_ndc_fs_in;                                 
}

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2) {
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}


vec4 interpolate4D(vec4 v0, vec4 v1, vec4 v2) {
    return vec4(gl_TessCoord.x) * v0 + vec4(gl_TessCoord.y) * v1 + vec4(gl_TessCoord.z) * v2;
}