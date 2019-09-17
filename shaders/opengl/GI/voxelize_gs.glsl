#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
} gs_in[];

struct GS_OUT
{
	vec4 pos;
	vec2 texCoords;
	vec3 N;
	vec3 P;
};

out GS_OUT gs_out;

layout(binding = 0) uniform Cbuffer {
    float       g_xFrame_VoxelRadianceDataSize;				// voxel half-extent in world space units
	float       g_xFrame_VoxelRadianceDataSize_rcp;			// 1.0 / voxel-half extent
    uint		g_xFrame_VoxelRadianceDataRes;				// voxel grid resolution
	float		g_xFrame_VoxelRadianceDataRes_rcp;			// 1.0 / voxel grid resolution
    
    uint		g_xFrame_VoxelRadianceDataMIPs;				// voxel grid mipmap count
	uint		g_xFrame_VoxelRadianceNumCones;				// number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceNumCones_rcp;			// 1.0 / number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceRayStepSize;			// raymarch step size in voxel space units
    
    vec3		g_xFrame_VoxelRadianceDataCenter;			// center of the voxel grid in world space units
	uint		g_xFrame_VoxelRadianceReflectionsEnabled;	// are voxel gi reflections enabled or not   
};


void main()
{

    GS_OUT vertexOuts[3];
    
    vec3 facenormal = abs(gs_in[0].normal + gs_in[1].normal + gs_in[2].normal);
	uint maxi = facenormal[1] > facenormal[0] ? 1 : 0;
	maxi = facenormal[2] > facenormal[maxi] ? 2 : maxi;
    
    for (uint i = 0; i < 3; ++i)
	{
		// World space -> Voxel grid space:
		vertexOuts[i].pos.xyz = (gs_in[i].position.xyz - g_xFrame_VoxelRadianceDataCenter) * g_xFrame_VoxelRadianceDataSize_rcp;

		// Project onto dominant axis:
		if (maxi == 0)
		{
			vertexOuts[i].pos.xyz = vertexOuts[i].pos.zyx;
		}
		else if (maxi == 1)
		{
			vertexOuts[i].pos.xyz = vertexOuts[i].pos.xzy;
		}
	}
    
    // Expand triangle to get fake Conservative Rasterization:
	vec2 side0N = normalize(vertexOuts[1].pos.xy - vertexOuts[0].pos.xy);
	vec2 side1N = normalize(vertexOuts[2].pos.xy - vertexOuts[1].pos.xy);
	vec2 side2N = normalize(vertexOuts[0].pos.xy - vertexOuts[2].pos.xy);
	vertexOuts[0].pos.xy += normalize(side2N - side0N);
	vertexOuts[1].pos.xy += normalize(side0N - side1N);
	vertexOuts[2].pos.xy += normalize(side1N - side2N);
    
    
    for (uint n = 0; n < gl_in.length(); ++n) {
        
        // Voxel grid space -> Clip space
        gs_out.pos.xy = vertexOuts[n].pos.xy * g_xFrame_VoxelRadianceDataRes_rcp;
		gs_out.pos.zw = vec2(1.0);
        gl_Position = gs_out.pos;

		// Append the rest of the parameters as is:
		gs_out.texCoords = gs_in[n].texCoords;
		gs_out.N = gs_in[n].normal;
		gs_out.P = gs_in[n].position.xyz;
        
        
        EmitVertex();
    }
    
    EndPrimitive();
}  