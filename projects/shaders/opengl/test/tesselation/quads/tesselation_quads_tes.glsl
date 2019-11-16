#version 450 core

layout(quads, equal_spacing, ccw) in; //equal_spacing

in vec2 texCoord_ndc_tes_in[];
out vec2 texCoord_ndc_fs_in;

//uniform mat4 transform;

void main() {
    
    
    const float u = gl_TessCoord.x;
    const float omu = 1 - u;
    const float v = gl_TessCoord.y;
    const float omv = 1 - v;
    
    const float bottomLeft = omu * omv; 
    const float bottomRight = u * omv;
    const float topRight = u*v; 
    const float topLeft = omu * v; 
    
    vec4 objectSpacePosition = bottomLeft * gl_in[0].gl_Position +
                  bottomRight * gl_in[1].gl_Position +
                  topRight * gl_in[2].gl_Position +
                  topLeft * gl_in[3].gl_Position;
                  
    gl_Position = objectSpacePosition;
                                        
                                        
    texCoord_ndc_fs_in = bottomLeft * texCoord_ndc_tes_in[0] +
                         bottomRight * texCoord_ndc_tes_in[1] + 
                         topRight * texCoord_ndc_tes_in[2] +
                         topLeft * texCoord_ndc_tes_in[3];                          
}