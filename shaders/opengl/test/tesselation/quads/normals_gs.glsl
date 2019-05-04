#version 450 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in TES_OUT{vec3 normal;} gs_in[];

const float MAGNITUDE = 0.4f;

void GenerateLine(int index)
{
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0f) * MAGNITUDE;
    //gl_Position = gl_in[index].gl_Position + vec4(0,1,0, 0.0f); //* MAGNITUDE;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // First vertex normal
    GenerateLine(1); // Second vertex normal
    GenerateLine(2); // Third vertex normal
}  