#include <pbr_deferred/HeightMap.hpp>
#include <nex/material/Material.hpp>
#include "nex/mesh/MeshFactory.hpp"

nex::HeightMap::HeightMap(unsigned xSegments, 
	unsigned zSegments, 
	float worldDimensionX,
	float worldDimensionZ,
	float worldDimensionMaxHeight) :
mXSegments(xSegments), 
mZSegments(zSegments), 
mWorldDimensionX(worldDimensionX), 
mWorldDimensionZ(worldDimensionZ),
mWorldDimensionMaxHeight(worldDimensionMaxHeight)
{
	// minimal one vertical and horizontal segment.
	mXSegments = std::max<unsigned>(mXSegments, 1);
	mZSegments = std::max<unsigned>(mZSegments, 1);

	const auto vertexXNumber = (xSegments + 1);
	const auto vertexZNumber = (zSegments + 1);
	const auto vertexNumber = vertexXNumber * vertexZNumber;

	/** 
	* We lay the vertices out into columns, going from bottom to top.
	* The y coordinate is always zero,
	* since it will be modified by the height map texture.
	* 
	* Example: 4x4 segments consisting of 5x5 vertices
	*  4__9_______
	*  3|_8|__|__|
	*  2|_7|__|__|
	*  1|_6|__|__|
	*  0|_5|__|__|
	*/
	std::vector<Vertex> vertices;
	vertices.resize(vertexNumber);

	for (unsigned x = 0; x < vertexXNumber; ++x)
	{
		for(unsigned z = 0; z < vertexZNumber; ++z)
		{
			Vertex& vertex = vertices[x*vertexZNumber + z];

			vertex.texCoords.x = (float)x / (float)mXSegments;
			vertex.texCoords.y = (float)z / (float)mZSegments;

			vertex.position.x = vertex.texCoords.x * mWorldDimensionX - mWorldDimensionX/2.0f; // scale and offset by world dimension
			vertex.position.y = 0.0f; // will be set by shader
			vertex.position.z = getZValue(vertex.texCoords.y * mWorldDimensionZ - mWorldDimensionZ / 2.0f);  // scale and offset by world dimension

			vertex.normal = glm::vec3(0,1,0); // normal points up
			vertex.tangent = glm::vec3(1,0,0); // tangent points to the right
			vertex.bitangent = glm::vec3(0,0, getZValue(1.0)); //bitangent points down
		}
	}

	// Next we construct indices forming quad patches in CCW
	std::vector<unsigned> indices;
	const auto quadNumber = xSegments * zSegments;
	const auto patchVertexCount = 4;
	indices.resize(quadNumber * patchVertexCount);

	for (unsigned x = 0; x < mXSegments; ++x)
	{
		for (unsigned z = 0; z < mZSegments; ++z)
		{
			const auto leftColumn = x * vertexZNumber;
			const auto rightColumn = (x+1) * vertexZNumber;

			const unsigned bottomLeft = leftColumn + z;
			const unsigned topLeft = leftColumn + z + 1;
			const unsigned topRight = rightColumn + z + 1;
			const unsigned bottomRight = rightColumn + z;
			
			const auto indexStart = (x * mZSegments + z)*patchVertexCount;

			//Note: CCW order
			indices[indexStart]   = bottomLeft;
			indices[indexStart+1] = bottomRight;
			indices[indexStart+2] = topRight;
			indices[indexStart+3] = topLeft;
		}
	}

	auto mesh = MeshFactory::create(vertices.data(), vertices.size(), indices.data(), indices.size());

	//TODO use a valid initialized material
	mMeshes.add(std::move(mesh), std::make_unique<Material>(nullptr));
}

nex::Mesh* nex::HeightMap::getMesh()
{
	return mMeshes.getMeshes()[0].get();
}