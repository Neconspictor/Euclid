#include <mesh/Mesh.hpp>

using namespace std;

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
	this->material = other.material;
}

Mesh::Mesh(Mesh&& other) : vertexData(other.vertexData),
	indexData(other.indexData)
{
	this->vertexSlice = other.vertexSlice;
	this->material = move(material);
}

Mesh& Mesh::operator=(const Mesh& o)
{
	this->indexData = o.indexData;
	this->material = o.material;
	this->vertexData = o.vertexData;
	this->vertexSlice = o.vertexSlice;
	return *this;
}

Mesh& Mesh::operator=(Mesh&& o)
{
	this->indexData = move(o.indexData);
	this->material = move(o.material);
	this->vertexData = move(o.vertexData);
	this->vertexSlice = move(o.vertexSlice);
	return *this;
}

Mesh::~Mesh()
{
}

const vector<size_t>& Mesh::getIndices() const
{
	return indexData;
}

const Material& Mesh::getMaterial() const
{
	return material;
}

const float* Mesh::getVertexData() const
{
	return vertexData.data();
}

size_t Mesh::getVertexSlice() const
{
	return vertexSlice;
}

void Mesh::setMaterial(Material material)
{
	this->material = move(material);
}

void Mesh::setContent(vector<float> vertices, size_t vertexSliceSize, vector<size_t> indices)
{
	this->vertexData = move(vertices);
	this->indexData = move(indices);
	this->vertexSlice = vertexSliceSize;
}