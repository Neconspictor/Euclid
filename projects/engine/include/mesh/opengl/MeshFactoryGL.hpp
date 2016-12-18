#pragma once
#include <mesh/opengl/MeshGL.hpp>

class MeshFactoryGL
{
public:
	
	/**
	 * The default mesh generation method. 
	 * Creates a gl mesh with position and normal data, and uv coordinates. 
	 */
	static MeshGL create(const VertexPositionNormalTex* vertices, uint32_t vertexCount, 
						 const uint32_t* indices, uint32_t indexCount);

	/**
	* Creates a gl mesh with position data.
	*/
	static MeshGL createPosition(const VertexPosition* vertices, uint32_t vertexCount,
		const uint32_t* indices, uint32_t indexCount);

	/**
	* Creates a gl mesh with position data and uv coordinates.
	*/
	static MeshGL createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount,
		const uint32_t* indices, uint32_t indexCount);
};