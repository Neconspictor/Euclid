#include <mesh/Mesh.hpp>

using namespace std;

Mesh::Mesh() : indexSize(0)
{
}

Mesh::Mesh(const Mesh& other)
{
	this->material = other.material;
	this->indexSize = other.indexSize;
}

Mesh::Mesh(Mesh&& other) : material(other.material), indexSize(other.indexSize)
{
}

Mesh& Mesh::operator=(const Mesh& o)
{
	this->material = o.material;
	indexSize = o.indexSize;
	return *this;
}

Mesh& Mesh::operator=(Mesh&& o)
{
	this->material = move(o.material);
	indexSize = move(o.indexSize);
	return *this;
}

Mesh::~Mesh()
{
}

Material* Mesh::getMaterial()
{
	return &material;
}

const Material& Mesh::getMaterial() const
{
	return material;
}

void Mesh::setIndexSize(uint32_t indexSize)
{
	this->indexSize = indexSize;
}

void Mesh::setMaterial(Material material)
{
	this->material = move(material);
}

unsigned Mesh::getIndexSize() const
{
	return indexSize;
}