#include <mesh/Mesh.hpp>

using namespace std;

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices)
{
	this->vertexData = move(vertices);
	this->indexData = move(indices);
}

Mesh::Mesh(const Mesh& other)
{
	this->vertexData = other.vertexData;
	this->indexData = other.indexData;
	this->material = other.material;
}

Mesh::Mesh(Mesh&& other) : vertexData(other.vertexData),
	indexData(other.indexData), material(other.material)
{
}

Mesh& Mesh::operator=(const Mesh& o)
{
	this->indexData = o.indexData;
	this->vertexData = o.vertexData;
	this->material = o.material;
	return *this;
}

Mesh& Mesh::operator=(Mesh&& o)
{
	this->indexData = move(o.indexData);
	this->vertexData = move(o.vertexData);
	this->material = move(o.material);
	return *this;
}

Mesh::~Mesh()
{
}

const vector<unsigned int>& Mesh::getIndices() const
{
	return indexData;
}

Material* Mesh::getMaterial()
{
	return &material;
}

const Material& Mesh::getMaterial() const
{
	return material;
}

const vector<Mesh::Vertex>& Mesh::getVertexData() const
{
	return vertexData;
}

void Mesh::setVertexData(const vector<Vertex>& vertices)
{
	this->vertexData = vertices;
}

void Mesh::setVertexData(vector<Vertex>&& vertices)
{
	vertexData = move(vertices);
}

void Mesh::setIndexData(const vector<unsigned>& indices)
{
	indexData = indices;
}

void Mesh::setIndexData(vector<unsigned>&& indices)
{
	indexData = move(indices);
}