#include <mesh/Mesh.hpp>

using namespace std;

Mesh::Mesh(): vertexSlice(0)
{
}

Mesh::Mesh(vector<float> vertices, size_t vertexSliceSize, vector<size_t> indices)
{
	this->vertexData = move(vertices);
	this->indexData = move(indices);
	this->vertexSlice = vertexSliceSize;
}

Mesh::Mesh(const Mesh& other)
{
	this->vertexData = other.vertexData;
	this->indexData = other.indexData;
	this->vertexSlice = other.vertexSlice;
}

Mesh::Mesh(Mesh&& other) : vertexData(other.vertexData),
	indexData(other.indexData)
{
	this->vertexSlice = other.vertexSlice;
}

Mesh::~Mesh()
{
}

const vector<size_t>& Mesh::getIndices() const
{
	return indexData;
}

const float* Mesh::getVertexData() const
{
	return vertexData.data();
}

size_t Mesh::getVertexSlice() const
{
	return vertexSlice;
}

void Mesh::setContent(vector<float> vertices, size_t vertexSliceSize, vector<size_t> indices)
{
	this->vertexData = move(vertices);
	this->indexData = move(indices);
	this->vertexSlice = vertexSliceSize;
}