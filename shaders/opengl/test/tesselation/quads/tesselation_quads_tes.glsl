#version 450 core

layout(quads, equal_spacing, ccw) in;

in vec2 texCoord_ndc_tes_in[];
out vec2 texCoord_ndc_fs_in;

void main() {
    
    
    const float u = gl_TessCoord.x;
    const float omu = 1 - u;
    const float v = gl_TessCoord.y;
    const float omv = 1 - v;
    
    const float factor0 = omu * omv;
    const float factor1 = u * omv;
    const float factor2 = u*v;
    const float factor3 = omu * v;
    
    gl_Position = factor0 * gl_in[0].gl_Position +
                  factor1 * gl_in[1].gl_Position +
                  factor2 * gl_in[2].gl_Position +
                  factor3 * gl_in[3].gl_Position;
                                        
                                        
    texCoord_ndc_fs_in = factor0 * texCoord_ndc_tes_in[0] +
                         factor1 * texCoord_ndc_tes_in[1] + 
                         factor2 * texCoord_ndc_tes_in[2] +
                         factor3 * texCoord_ndc_tes_in[3];                          
}