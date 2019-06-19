#include <nex/mesh/Mesh.hpp>
#include "VertexArray.hpp"
#include "IndexBuffer.hpp"
#include "nex/material/Material.hpp"

using namespace std;
using namespace nex;

Mesh::Mesh(VertexArray vertexArray, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, AABB boundingBox, Topology topology) :
mVertexArray(std::move(vertexArray)),
mVertexBuffer(std::move(vertexBuffer)),
mIndexBuffer(std::move(indexBuffer)),
mBoundingBox(std::move(boundingBox)),
mTopology(topology)
{
}

void Mesh::setVertexBuffer(VertexBuffer buffer)
{
	mVertexBuffer = std::move(buffer);
}

Mesh::Mesh(): mTopology(Topology::TRIANGLES)
{
	//mBoundingBox.min = glm::vec3(0.0f);
	//mBoundingBox.max = glm::vec3(0.0f);
}

const AABB& Mesh::getAABB() const
{
	return mBoundingBox;
}

void Mesh::setBoundingBox(const AABB& box)
{
	mBoundingBox = box;
}

IndexBuffer* Mesh::getIndexBuffer()
{
	return &mIndexBuffer;
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