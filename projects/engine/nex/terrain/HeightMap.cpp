#include <nex/terrain/HeightMap.hpp>
#include <nex/material/Material.hpp>
#include "nex/mesh/MeshFactory.hpp"
#include "nex/texture/Texture.hpp"
#include "nex/texture/Sampler.hpp"
#include <nex/math/Math.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/shader/ShaderProvider.hpp>

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

	mMeshes = std::make_unique<MeshGroup>();

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
		}
	}

	// calc normal, tangent, bitangent
	for (int row = 0; row < mRows; ++row)
	{
		for (int column = 0; column < mColumns; ++column)
		{
			generateTBN(vertices, row, column);
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

	AABB boundingBox;
	boundingBox.min = glm::vec3(-mWorldDimensionX / 2.0f, 0.0f, -mWorldDimensionZ / 2.0f);
	boundingBox.max = glm::vec3(mWorldDimensionX / 2.0f, mWorldDimensionMaxHeight, mWorldDimensionZ / 2.0f);
	auto mesh = MeshFactory::create(vertices.data(), vertices.size(), indices.data(), indices.size(), std::move(boundingBox));

	//TODO use a valid initialized material
	mMeshes->add(std::move(mesh), std::make_unique<Material>(std::make_shared<ShaderProvider>(nullptr)));
	mMeshes->calcBatches();

	mMeshes->finalize();


	TextureDesc heightDesc;
	heightDesc.colorspace = ColorSpace::R;
	heightDesc.internalFormat = InternalFormat::R32F;
	heightDesc.pixelDataType = PixelDataType::FLOAT;

	mHeightTexture = std::make_unique<Texture2D>(mRows, mColumns, heightDesc, heights.data());

	SamplerDesc heightSamplerDesc;
	heightSamplerDesc.minFilter = TexFilter::Nearest;
	heightSamplerDesc.magFilter = TexFilter::Nearest;
	heightSamplerDesc.wrapR = heightSamplerDesc.wrapS = heightSamplerDesc.wrapT = UVTechnique::ClampToEdge;
	// we specify a negative border, so that we can detect out of range sampling in shaders
	//heightSamplerDesc.borderColor = glm::vec4(-1.0f);

	mHeightSampler = std::make_unique<Sampler>(heightSamplerDesc);
}

nex::HeightMap::~HeightMap() = default;

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
		if (worldDimensionMaxHeight != 0.0f)
		{
			float d = std::rand() / ((RAND_MAX + 1u) / worldDimensionMaxHeight);
			heights[i] = d / worldDimensionMaxHeight;
		} else
		{
			heights[i] = 0.0f;
		}
	}

	return HeightMap(rows, columns, worldDimensionZ, worldDimensionMaxHeight, worldDimensionX, heights);
}

nex::Mesh* nex::HeightMap::getMesh()
{
	return mMeshes->getEntries()[0].get();
}

nex::Sampler* nex::HeightMap::getHeightSampler()
{
	return mHeightSampler.get();
}

nex::Texture2D* nex::HeightMap::getHeightTexture()
{
	return mHeightTexture.get();
}

glm::vec3 nex::HeightMap::getWorldDimension() const
{
	return glm::vec3(mWorldDimensionX, mWorldDimensionMaxHeight, mWorldDimensionZ);
}

glm::uvec2 nex::HeightMap::getVertexCount() const
{
	return glm::uvec2(mColumns, mRows);
}

void nex::HeightMap::generateTBN(std::vector<Vertex>& vertices, int row, int column) const
{

	auto& vertex = vertices[getIndex(row, column)];

	// sample the nearest enivorment (8 surrounding vertices)
	// Note: We add them in CCW; this way it will be easier to define triangles in CCW for correct normal direction
	std::vector<unsigned> environment;

	// bottom left
	if (isInRange(row - 1, column - 1)) environment.push_back(getIndex(row - 1, column - 1));
	// bottom
	if (isInRange(row - 1, column)) environment.push_back(getIndex(row - 1, column));
	// bottom right
	if (isInRange(row - 1, column + 1)) environment.push_back(getIndex(row - 1, column + 1));
	// right
	if (isInRange(row, column + 1)) environment.push_back(getIndex(row, column + 1));
	// top right
	if (isInRange(row + 1, column + 1)) environment.push_back(getIndex(row + 1, column + 1));
	// top
	if (isInRange(row + 1, column)) environment.push_back(getIndex(row + 1, column));
	// top left
	if (isInRange(row + 1, column - 1)) environment.push_back(getIndex(row + 1, column - 1));
	// left
	if (isInRange(row, column - 1)) environment.push_back(getIndex(row, column - 1));


	// To get the tbn's of the environment, we define triangles in a circle around the middle point
	// Note: this method doesn't match the real defined geometry, but is fine for smooth normals in this special case.
	glm::vec3 accumulatedNormal(0.0f);
	glm::vec3 accumulatedTangent(0.0f);
	glm::vec3 accumulatedBitangent(0.0f);

	auto averageSize = environment.size();
	
	for (unsigned i = 0; i < environment.size(); ++i)
	{
		const unsigned nextI = (i + 1) % environment.size();
		// note that we use unnormalized normal vectors, so that triangles with more area influence the result more strongly.
		const auto& current = vertices[environment[i]];
		const auto& next = vertices[environment[nextI]];
		const auto tbn = calcTBN(vertex, current, next);

		//if the vertex, current and next lie on a line, the tbn isn't valid -> skip it
		if (isValidTBN(tbn))
		{
			accumulatedNormal += tbn.normal;
			accumulatedTangent += tbn.tangent;
			accumulatedBitangent += tbn.bitangent;
		} else
		{
			--averageSize;
		}
	}

	averageSize = max(averageSize, 1);
	vertex.normal = glm::normalize(accumulatedNormal / (float)averageSize);
	vertex.tangent = glm::normalize(accumulatedTangent / (float)averageSize);
	auto bitangent = glm::normalize(accumulatedBitangent / (float)averageSize);


	// Gram-Schmidt orthogonalize
	vertex.tangent = normalize((vertex.tangent - bitangent * dot(vertex.normal, vertex.tangent)));

	// Calculate handiness and recalculate bitangent
	//const float handiness = (dot(cross(vertex.normal, vertex.tangent), bitangent) < 0.0F) ? -1.0f : 1.0f;
	//vertex.bitangent = normalize(handiness * cross(vertex.normal, vertex.tangent));

	// TODO store handiness in a 4D tangent vector and remove the need of the bitangent!
	//tangent[i].w = (dot(cross(n, t), bitangents[i]) < 0.0F) ? -1.0f : 1.0f;
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

bool nex::HeightMap::isInRange(int row, int column) const
{
	return  !(row < 0 || row >= mRows || column < 0 || column >= mColumns);
}

bool nex::HeightMap::isValidTBN(const TBN& tbn)
{
	if (length(tbn.normal) == 0.0f) return false;
	if (std::isnan(tbn.tangent.x)) return false;
	if (std::isnan(tbn.bitangent.x)) return false;
	return true;
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
	return glm::cross(b - a, c - a);
}

nex::HeightMap::TBN nex::HeightMap::calcTBN(const Vertex& a, const Vertex& b, const Vertex& c)
{
	TBN result;
	result.normal = calcNormal(a.position, b.position, c.position);

	const auto vecAB = b.position - a.position;
	const auto vecAC = c.position - a.position;

	const auto uvAB = b.texCoords - a.texCoords;
	const auto uvAC = c.texCoords - a.texCoords;

	
	/**
	 * For math derivation: Mathematics for 3D Game Programming and Computer Graphics (Third Edition), chapter 7.8.3, p. 180, Eric Lengyel
	 */
	float r = 1.0f / (uvAB.x * uvAC.y - uvAC.x * uvAB.y);
	result.tangent = glm::vec3((uvAC.y * vecAB.x - uvAB.y * vecAC.x) * r,
		(uvAC.y * vecAB.y - uvAB.y * vecAC.y) * r,
		(uvAC.y * vecAB.z - uvAB.y * vecAC.z) * r);

	result.bitangent = glm::vec3((uvAB.x * vecAC.x - uvAC.x * vecAB.x) * r,
		(uvAB.x * vecAC.y - uvAC.x * vecAB.y) * r,
		(uvAB.x * vecAC.z - uvAC.x * vecAB.z) * r);

	return result;
}