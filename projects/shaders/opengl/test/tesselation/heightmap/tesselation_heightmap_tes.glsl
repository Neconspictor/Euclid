#version 460 core

layout(quads, equal_spacing, ccw) in; //equal_spacing

in TCS_OUT {
    vec3 normal;
    vec2 texCoords;
} tcs_out[];

out TES_OUT {
    vec3 normal;
    vec2 texCoords;
} tes_out;

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
                                        
                                        
                                        
    tes_out.normal = bottomLeft * tcs_out[0].normal +
                     bottomRight * tcs_out[1].normal + 
                     topRight * tcs_out[2].normal +
                     topLeft * tcs_out[3].normal;                                     
                                        
    tes_out.texCoords =  bottomLeft * tcs_out[0].texCoords +
                         bottomRight * tcs_out[1].texCoords + 
                         topRight * tcs_out[2].texCoords +
                         topLeft * tcs_out[3].texCoords;                          
}