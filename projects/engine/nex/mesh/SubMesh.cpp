#include <nex/mesh/SubMesh.hpp>
#include "VertexArray.hpp"
#include "IndexBuffer.hpp"
#include "nex/material/Material.hpp"

using namespace std;
using namespace nex;

SubMesh::SubMesh(VertexArray vertexArray, IndexBuffer indexBuffer, Material* material) :
mVertexArray(std::move(vertexArray)),
mIndexBuffer(std::move(indexBuffer)),
mMaterial(material)
{
}

SubMesh::SubMesh(): mMaterial(nullptr)
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

VertexArray* SubMesh::getVertexArray()
{
	return &mVertexArray;
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