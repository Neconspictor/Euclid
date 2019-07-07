#version 420

layout(location = 0)in vec2 Position;
layout(location = 1)in vec2 UV;
layout(location = 2)in vec4 Color;
out vec2 Frag_UV;
out vec4 Frag_Color;

uniform mat4 ProjMtx;
uniform mat3 transformUV;
uniform int FlipY;

void main()
{
    Frag_UV = vec2(transformUV * vec3(UV, 1.0));
    
    //if (FlipY) {
    //    Frag_UV.y = -Frag_UV.y + 1.0;
   // }

    Frag_Color = Color;
    gl_Position = ProjMtx * vec4(Position.xy,0,1);
}