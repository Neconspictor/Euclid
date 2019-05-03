#include <pbr_deferred/HeightMap.hpp>
#include <nex/material/Material.hpp>
#include "nex/mesh/MeshFactory.hpp"

nex::HeightMap::HeightMap(unsigned xSegments, 
	unsigned zSegments, 
	float worldDimensionX,
	float worldDimensionZ,
	float worldDimensionMaxHeight,
	const std::vector<float>& heights) :
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

	if (heights.size() != vertexNumber)
	{
		throw_with_trace(std::runtime_error("heights doesn't have (xSegments + 1)*(zSegments + 1) values!"));
	}

	/** 
	* We lay the vertices out into rows, going from bottom to top.
	*/
	std::vector<Vertex> vertices;
	vertices.resize(vertexNumber);

	for (unsigned z = 0; z < vertexZNumber; ++z)
	{
		for(unsigned x = 0; x < vertexXNumber; ++x)
		{
			const auto index = z * vertexXNumber + x;
			Vertex& vertex = vertices[index];

			vertex.texCoords.x = (float)x / (float)mXSegments;
			vertex.texCoords.y = (float)z / (float)mZSegments;

			vertex.position.x = vertex.texCoords.x * mWorldDimensionX - mWorldDimensionX/2.0f; // scale and offset by world dimension
			vertex.position.y = heights[index] * mWorldDimensionMaxHeight;
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

	for (unsigned z = 0; z < mZSegments; ++z)
	{
		for (unsigned x = 0; x < mXSegments; ++x)
		{
			const auto bottomRow = z * vertexXNumber;
			const auto topRow = (z+1) * vertexXNumber;

			const unsigned bottomLeft = bottomRow + x;
			const unsigned topLeft = topRow + x;
			const unsigned topRight = topRow + x + 1;
			const unsigned bottomRight = bottomRow + x + 1;
			
			const auto indexStart = (z * mXSegments + x)*patchVertexCount;

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

nex::HeightMap nex::HeightMap::createZero(unsigned xSegments, unsigned zSegments, float worldDimensionX,
	float worldDimensionZ)
{
	std::vector<float> heights((xSegments + 1) * (zSegments + 1), 0.0f);
	return HeightMap(xSegments, zSegments, worldDimensionX, worldDimensionZ, 0.0f, heights);
}

nex::HeightMap nex::HeightMap::createRandom(unsigned xSegments, unsigned zSegments, float worldDimensionX,
	float worldDimensionZ, float worldDimensionMaxHeight)
{
	std::vector<float> heights((xSegments + 1) * (zSegments + 1));

	for (unsigned i = 0; i < heights.size(); ++i)
	{
		float d = std::rand() / ((RAND_MAX + 1u) / worldDimensionMaxHeight);

		heights[i] = d/worldDimensionMaxHeight;
	}

	return HeightMap(xSegments, zSegments, worldDimensionX, worldDimensionZ, worldDimensionMaxHeight, heights);
}

nex::Mesh* nex::HeightMap::getMesh()
{
	return mMeshes.getMeshes()[0].get();
}
