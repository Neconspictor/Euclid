#version 330 core
in VS_OUT {
    vec2 tex;
    
} gs_in[];

out GS_OUT {
    vec2 tex;
} gs_out;

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

uniform float time;


vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 1.0f;
    vec3 direction = normal * (sin(time)) * magnitude; 
    return position + vec4(direction, 0.0f);
} 


vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}  



void main() {
    vec3 normal = GetNormal();
    int i;
    for ( i=0; i < gl_in.length(); i++) {
        gs_out.tex = gs_in[i].tex;
        gl_Position = explode(gl_in[i].gl_Position, normal);
        EmitVertex();
    }
    EndPrimitive();
} 