#version 430 core

#ifndef C_UNIFORM_BUFFER_BINDING_POINT
#define C_UNIFORM_BUFFER_BINDING_POINT 0
#endif


layout (points) in;
layout (triangle_strip, max_vertices = 36) out;

layout(std140, binding = C_UNIFORM_BUFFER_BINDING_POINT) uniform Cbuffer {
    float       g_xFrame_VoxelRadianceDataSize;				// voxel half-extent in world space units
	float       g_xFrame_VoxelRadianceDataSize_rcp;			// 1.0 / voxel-half extent
    uint		g_xFrame_VoxelRadianceDataRes;				// voxel grid resolution
	float		g_xFrame_VoxelRadianceDataRes_rcp;			// 1.0 / voxel grid resolution
    
    uint		g_xFrame_VoxelRadianceDataMIPs;				// voxel grid mipmap count
	uint		g_xFrame_VoxelRadianceNumCones;				// number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceNumCones_rcp;			// 1.0 / number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceRayStepSize;			// raymarch step size in voxel space units
    
    vec4		g_xFrame_VoxelRadianceDataCenter;			// center of the voxel grid in world space units
	uint		g_xFrame_VoxelRadianceReflectionsEnabled;	// are voxel gi reflections enabled or not   
};

uniform mat4 viewProj;

in VS_OUT {
    vec3 positionWS;
    vec4 color;
} gs_in[];

out GS_OUT {
    vec4 color;
} gs_out;


void createVertex(const in vec4 p) {
    gs_out.color = gs_in[0].color;
    gl_Position = p;
    EmitVertex();
}


void createTriangle(const in vec4 p1, const in vec4 p2, const in vec4 p3) {

    createVertex(p1);
    createVertex(p2);
    createVertex(p3);
    EndPrimitive();
}

const vec3 xAxis = vec3(1,0,0);
const vec3 yAxis = vec3(0,1,0);
const vec3 zAxis = vec3(0,0,-1); // Remember: Right handed (important for winding order)!

void main()
{

    if (gs_in[0].color.a < 0.001) {
        return;
    };

    vec3 middle = gs_in[0].positionWS;
    float halfVoxelSize = g_xFrame_VoxelRadianceDataSize;
    
    vec3 voxelX = xAxis * halfVoxelSize;
    vec3 voxelY = yAxis * halfVoxelSize;
    vec3 voxelZ = zAxis * halfVoxelSize;
    
    // 8 vertices of the cube
    vec4 leftBottomBack = viewProj * vec4(middle - voxelX - voxelY - voxelZ, 1.0);
    vec4 leftBottomFront = viewProj * vec4(middle - voxelX - voxelY + voxelZ, 1.0);
    vec4 leftTopBack = viewProj * vec4(middle - voxelX + voxelY - voxelZ, 1.0);
    vec4 leftTopFront = viewProj * vec4(middle - voxelX + voxelY + voxelZ, 1.0);
    
    vec4 rightBottomBack = viewProj * vec4(middle + voxelX - voxelY - voxelZ, 1.0);
    vec4 rightBottomFront = viewProj * vec4(middle + voxelX - voxelY + voxelZ, 1.0);
    vec4 rightTopBack = viewProj * vec4(middle + voxelX + voxelY - voxelZ, 1.0);
    vec4 rightTopFront = viewProj * vec4(middle + voxelX + voxelY + voxelZ, 1.0);
    
    //left side
    createTriangle(leftBottomBack, leftBottomFront, leftTopFront);
    createTriangle(leftBottomBack, leftTopFront, leftTopBack);
    
    //right side
    createTriangle(rightBottomBack, rightTopBack, rightTopFront);
    createTriangle(rightBottomBack, rightTopFront, rightBottomFront);
    
    //top side
    createTriangle(leftTopFront, rightTopFront, rightTopBack);
    createTriangle(leftTopFront, rightTopBack, leftTopBack);
    
    //bottom side
    createTriangle(leftBottomBack, rightBottomBack, rightBottomFront);
    createTriangle(leftBottomBack, rightBottomFront, leftBottomFront);
    
    //front side
    createTriangle(leftBottomFront, rightBottomFront, rightTopFront);
    createTriangle(leftBottomFront, rightTopFront, leftTopFront);
    
    //back side
    createTriangle(rightBottomBack, leftBottomBack, leftTopBack);
    createTriangle(rightBottomBack, leftTopBack, rightTopBack);
}  