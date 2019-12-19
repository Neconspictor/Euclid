#version 460 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in TES_OUT{
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 positionViewSpace;
} gs_in[];

out GS_OUT{
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 color;
} fs_in;


uniform mat4 projection;

const float MAGNITUDE = 0.3f;

void GenerateNormal(int index)
{
    gl_Position = gl_in[index].gl_Position;
    fs_in.color = vec3(0,0,1);
    EmitVertex();
    gl_Position = projection * (vec4(gs_in[index].positionViewSpace, 1.0) 
                                + vec4(gs_in[index].normal, 0.0f) * MAGNITUDE);
    //gl_Position = gl_in[index].gl_Position + vec4(0,1,0, 0.0f); //* MAGNITUDE;
    EmitVertex();
    EndPrimitive();
}

void GenerateTangent(int index)
{
    gl_Position = gl_in[index].gl_Position;
    fs_in.color = vec3(1,0,0);
    EmitVertex();
    gl_Position = projection * (vec4(gs_in[index].positionViewSpace, 1.0) 
                                + vec4(gs_in[index].tangent, 0.0f) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void GenerateBitangent(int index)
{
    gl_Position = gl_in[index].gl_Position;
    fs_in.color = vec3(0,1,0);
    EmitVertex();
    gl_Position = projection * (vec4(gs_in[index].positionViewSpace, 1.0) 
                                + vec4(gs_in[index].bitangent, 0.0f) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateNormal(0); // First vertex
    GenerateTangent(0);
    GenerateBitangent(0);
    GenerateNormal(1); // Second vertex
    GenerateTangent(1);
    GenerateBitangent(1);
    GenerateNormal(2); // Third vertex
    GenerateTangent(2);
    GenerateBitangent(2);
}  