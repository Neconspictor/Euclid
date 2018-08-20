#include <nex/mesh/Mesh.hpp>

using namespace std;

Mesh::Mesh() : indexSize(0)
{
}

Mesh::Mesh(Mesh&& other) : indexSize(other.indexSize)
{
	material = move(other.material);
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

reference_wrapper<Material> Mesh::getMaterial() const
{
	return std::ref(*material);
}

void Mesh::setIndexSize(uint32_t indexSize)
{
	this->indexSize = indexSize;
}

void Mesh::setMaterial(std::unique_ptr<Material> material)
{
	this->material = move(material);
}

unsigned Mesh::getIndexSize() const
{
	return indexSize;
}