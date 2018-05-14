#pragma once
#include <mesh/MeshGL.hpp>

class MeshFactoryGL
{
public:

	template<typename type>
	struct SimpleArray
	{
		const type* content;
		uint32_t size;
	};
	
	/**
	 * The default mesh generation method. 
	 * Creates a gl mesh with position and normal data, and uv coordinates. 
	 */
	static std::unique_ptr<MeshGL> create(const VertexPositionNormalTexTangent* vertices, uint32_t vertexCount,
						 const uint32_t* indices, uint32_t indexCount);

	static std::unique_ptr<MeshGL> create(const VertexPositionNormalTex* vertices, uint32_t vertexCount,
		const uint32_t* indices, uint32_t indexCount);

	static std::unique_ptr<MeshGL> create(SimpleArray<glm::vec3> positions, SimpleArray<glm::vec3> normals,
		SimpleArray<glm::vec2> texCoords, SimpleArray<unsigned int> indices);

	/**
	* Creates a gl mesh with position data.
	*/
	static std::unique_ptr<MeshGL> createPosition(const VertexPosition* vertices, uint32_t vertexCount,
		const uint32_t* indices, uint32_t indexCount);

	/**
	* Creates a gl mesh with position data and uv coordinates.
	*/
	static std::unique_ptr<MeshGL> createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount,
		const uint32_t* indices, uint32_t indexCount);
};