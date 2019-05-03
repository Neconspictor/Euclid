#include <pbr_deferred/HeightMap.hpp>
#include <nex/material/Material.hpp>
#include "nex/mesh/MeshFactory.hpp"
#include <numeric>

nex::HeightMap::HeightMap(unsigned rows,
	unsigned columns, 
	float worldDimensionZ,
	float worldDimensionMaxHeight,
	float worldDimensionX,
	const std::vector<float>& heights) :
mColumns(columns),
mRows(rows), 
mWorldDimensionX(worldDimensionX), 
mWorldDimensionZ(worldDimensionZ),
mWorldDimensionMaxHeight(worldDimensionMaxHeight)
{
	if (mColumns < 2)
	{
		throw_with_trace(std::runtime_error("nex::HeightMap::HeightMap: columns has to be >= 2"));
	}

	if (mRows < 2)
	{
		throw_with_trace(std::runtime_error("nex::HeightMap::HeightMap: rows has to be >= 2"));
	}

	const auto vertexNumber = mRows * mColumns;

	if (heights.size() != vertexNumber)
	{
		throw_with_trace(std::runtime_error("nex::HeightMap::HeightMap: heights doesn't have rows * columns values!"));
	}

	/** 
	* We lay the vertices out into rows, going from bottom to top.
	*/
	std::vector<Vertex> vertices;
	vertices.resize(vertexNumber);

	for (int row = 0; row < mRows; ++row)
	{
		for(int column = 0; column < mColumns; ++column)
		{
			const auto index = getIndex(row, column);
			Vertex& vertex = vertices[index];

			vertex.texCoords.x = (float)column / (float)(mColumns - 1);
			vertex.texCoords.y = (float)row / (float)(mRows - 1);
			
			vertex.position = calcPosition(row, column, heights);

			generateTBN(vertex, heights, row, column);
		}
	}

	// Next we construct indices forming quad patches in CCW
	std::vector<unsigned> indices;
	const auto quadNumber = (mRows-1) * (mColumns-1);
	const auto patchVertexCount = 4;
	indices.resize(quadNumber * patchVertexCount);

	for (int row = 0; row < (mRows-1); ++row)
	{
		for (int column = 0; column < (mColumns - 1); ++column)
		{
			const auto bottomLeft = getIndex(row, column);
			const auto topLeft = getIndex(row + 1, column);
			const auto topRight = getIndex(row + 1, column + 1);
			const auto bottomRight = getIndex(row, column + 1);
			
			const auto indexStart = (row * (mRows-1) + column)*patchVertexCount;

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

nex::HeightMap nex::HeightMap::createZero(unsigned rows, unsigned columns, float worldDimensionZ,
	float worldDimensionX)
{
	std::vector<float> heights(rows * columns, 0.0f);
	return HeightMap(rows, columns, worldDimensionZ, 0.0f, worldDimensionX, heights);
}

nex::HeightMap nex::HeightMap::createRandom(unsigned rows, unsigned columns, float worldDimensionZ,
	float worldDimensionMaxHeight, float worldDimensionX)
{
	std::vector<float> heights(rows * columns);

	for (unsigned i = 0; i < heights.size(); ++i)
	{
		float d = std::rand() / ((RAND_MAX + 1u) / worldDimensionMaxHeight);

		heights[i] = d/worldDimensionMaxHeight;
	}

	return HeightMap(rows, columns, worldDimensionZ, worldDimensionMaxHeight, worldDimensionX, heights);
}

nex::Mesh* nex::HeightMap::getMesh()
{
	return mMeshes.getMeshes()[0].get();
}

void nex::HeightMap::generateTBN(Vertex& vertex, const std::vector<float>& heights, int row, int column) const
{
	// sample the nearest enivorment (8 surrounding vertices)
	// Note: We add them in CCW; this way it will be easier to define triangles in CCW for correct normal direction
	std::vector<glm::vec3> environment;

	// bottom left
	addPosition(environment, heights, row - 1, column - 1);
	// bottom
	addPosition(environment, heights, row-1, column);
	// bottom right
	addPosition(environment, heights, row - 1, column + 1);
	// right
	addPosition(environment, heights, row, column + 1);
	// top right
	addPosition(environment, heights, row + 1, column + 1);
	// top
	addPosition(environment, heights, row + 1, column);
	// top left
	addPosition(environment, heights, row + 1, column - 1);
	// left
	addPosition(environment, heights, row, column - 1);


	// To get the tbn's of the environment, we define triangles in a circle around the middle point
	// Note: this method doesn't match the real defined geometry, but is fine for smooth normals.
	std::vector<glm::vec3> normals;
	const glm::vec3& middlePoint = vertex.position;
	for (unsigned i = 0; i < environment.size(); ++i)
	{
		const unsigned next = (i + 1) % environment.size();
		normals.emplace_back(calcNormal(middlePoint, environment[i], environment[next]));
	}


	glm::vec3 accNormal = std::accumulate(normals.begin(), normals.end(), glm::vec3(0.0f), [](const glm::vec3& accumulated, const glm::vec3& current)
	{
		return accumulated + current;
	});

	vertex.normal = glm::normalize(accNormal / (float)normals.size());

	//TODO tangent/bitangent!!!
	vertex.tangent = glm::vec3(1, 0, 0); // tangent points to the right
	vertex.bitangent = glm::vec3(0, 0, getZValue(1.0)); //bitangent points down
}

int nex::HeightMap::getColumn(int index) const
{
	const auto row = getRow(index);
	return index - row * mColumns;
}

int nex::HeightMap::getIndex(int row, int column) const
{
	return row * mColumns + column;
}

int nex::HeightMap::getRow(int index) const
{
	return index / (float)mColumns;
}

bool nex::HeightMap::sample(const std::vector<float>& heights, int row, int column, glm::vec3& out) const
{
	if (row < 0 || 
		row >= mRows ||
		column < 0 ||
		column >= mColumns)
	{
		return false;
	}

	out = calcPosition(row, column, heights);

	return true;
}

float nex::HeightMap::sampleHeight(const std::vector<float>& heights, int row, int column) const
{
	assert(row >= 0 && column >= 0);
	return heights[getIndex(row, column)];
}

void nex::HeightMap::addPosition(std::vector<glm::vec3>& vec, const std::vector<float>& heights,
	int row, int column) const
{
	glm::vec3 position;
	if (sample(heights, row, column, position))
	{
		vec.emplace_back(std::move(position));
	}
}

glm::vec3 nex::HeightMap::calcPosition(float row, float column, const std::vector<float>& heights) const
{
	const auto xNormalized = (float)column / (float)(mColumns - 1);
	const auto zNormalized = (float)row / (float)(mRows - 1);
	const auto height = sampleHeight(heights, row, column);

	return glm::vec3( xNormalized * mWorldDimensionX - mWorldDimensionX / 2.0f,
					  height * mWorldDimensionMaxHeight,
					  getZValue(zNormalized * mWorldDimensionZ - mWorldDimensionZ / 2.0f));
}

glm::vec3 nex::HeightMap::calcNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	return glm::normalize(glm::cross(b - a, c - a));
}