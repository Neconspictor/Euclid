#version 330 core
layout (location = 0) in vec2 position;

uniform mat4 viewProj;
uniform mat3 invView3x3;
uniform mat4 model;

out vec2 texCoordsFS;

void main()
{    
    vec4 positionLocal = vec4(position, 0.0, 1.0);    
    //vec4 positionWS = vec4(invView3x3 * positionLocal.xyz, 1.0);//positionLocal + vec4(vec3(model[3]), 0.0);
    //positionWS.xyz += vec3(model[3]);
    //positionWS += vec4(0.0, position.y, 0.0, 0.0);
    //positionWS += vec4(invView3x3[0] * position.x, 0.0);

    gl_Position = viewProj * model * positionLocal; 
    texCoordsFS = position + 0.5;
} 