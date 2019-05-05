#version 450 core

layout(quads, equal_spacing, ccw) in; //equal_spacing

in  TCS_OUT{
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 positionViewSpace;
} tes_in[];

out TES_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 positionViewSpace;
} gs_in;

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

    gs_in.tangent = bottomLeft * tes_in[0].tangent +
                         bottomRight * tes_in[1].tangent + 
                         topRight * tes_in[2].tangent +
                         topLeft * tes_in[3].tangent;

    gs_in.bitangent = bottomLeft * tes_in[0].bitangent +
                         bottomRight * tes_in[1].bitangent + 
                         topRight * tes_in[2].bitangent +
                         topLeft * tes_in[3].bitangent;

    gs_in.positionViewSpace = bottomLeft * tes_in[0].positionViewSpace +
                         bottomRight * tes_in[1].positionViewSpace + 
                         topRight * tes_in[2].positionViewSpace +
                         topLeft * tes_in[3].positionViewSpace;                         
}