#version 450 core

layout(quads, equal_spacing, ccw) in; //equal_spacing

in  TCS_OUT{vec3 normal;} tes_in[];
out TES_OUT{vec3 normal;} gs_in;

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
                                        
                                        
    gs_in.normal = bottomLeft * tes_in[0].normal +
                         bottomRight * tes_in[1].normal + 
                         topRight * tes_in[2].normal +
                         topLeft * tes_in[3].normal;                          
}