#include <nex/mesh/SubMesh.hpp>
#include "VertexArray.hpp"
#include "IndexBuffer.hpp"
#include "nex/material/Material.hpp"

using namespace std;
using namespace nex;

SubMesh::SubMesh(VertexArray vertexArray, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, Topology topology, Material* material) :
mVertexArray(std::move(vertexArray)),
mVertexBuffer(std::move(vertexBuffer)),
mIndexBuffer(std::move(indexBuffer)),
mMaterial(material),
mTopology(topology)
{
}

void SubMesh::setVertexBuffer(VertexBuffer buffer)
{
	mVertexBuffer = std::move(buffer);
}

SubMesh::SubMesh(): mMaterial(nullptr), mTopology(Topology::TRIANGLES)
{
}

IndexBuffer* SubMesh::getIndexBuffer()
{
	return &mIndexBuffer;
}

Material* SubMesh::getMaterial() const
{
	return mMaterial;
}

Topology SubMesh::getTopology() const
{
	return mTopology;
}

void SubMesh::setTopology(Topology topology)
{
	mTopology = topology;
}

VertexArray* SubMesh::getVertexArray()
{
	return &mVertexArray;
}

VertexBuffer* SubMesh::getVertexBuffer()
{
	return &mVertexBuffer;
}

void SubMesh::setIndexBuffer(IndexBuffer buffer)
{
	mIndexBuffer = std::move(buffer);
}

void SubMesh::setVertexArray(VertexArray vertexArray)
{
	mVertexArray = std::move(vertexArray);
}

void SubMesh::setMaterial(Material* material)
{
	mMaterial = material;
}