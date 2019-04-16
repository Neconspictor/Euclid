#include <nex/mesh/SubMesh.hpp>
#include "VertexArray.hpp"
#include "IndexBuffer.hpp"
#include "nex/material/Material.hpp"

using namespace std;
using namespace nex;

Mesh::Mesh(VertexArray vertexArray, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, Topology topology, Material* material) :
mVertexArray(std::move(vertexArray)),
mVertexBuffer(std::move(vertexBuffer)),
mIndexBuffer(std::move(indexBuffer)),
mMaterial(material),
mTopology(topology)
{
}

void Mesh::setVertexBuffer(VertexBuffer buffer)
{
	mVertexBuffer = std::move(buffer);
}

Mesh::Mesh(): mMaterial(nullptr), mTopology(Topology::TRIANGLES)
{
}

IndexBuffer* Mesh::getIndexBuffer()
{
	return &mIndexBuffer;
}

Material* Mesh::getMaterial() const
{
	return mMaterial;
}

Topology Mesh::getTopology() const
{
	return mTopology;
}

void Mesh::setTopology(Topology topology)
{
	mTopology = topology;
}

VertexArray* Mesh::getVertexArray()
{
	return &mVertexArray;
}

VertexBuffer* Mesh::getVertexBuffer()
{
	return &mVertexBuffer;
}

void Mesh::setIndexBuffer(IndexBuffer buffer)
{
	mIndexBuffer = std::move(buffer);
}

void Mesh::setVertexArray(VertexArray vertexArray)
{
	mVertexArray = std::move(vertexArray);
}

void Mesh::setMaterial(Material* material)
{
	mMaterial = material;
}