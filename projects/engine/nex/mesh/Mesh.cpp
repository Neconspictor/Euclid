#include <nex/mesh/Mesh.hpp>
#include "nex/material/Material.hpp"
#include <nex/resource/ResourceLoader.hpp>

using namespace std;
using namespace nex;

Mesh::Mesh() : mTopology(Topology::TRIANGLES)
{
	//mBoundingBox.min = glm::vec3(0.0f);
	//mBoundingBox.max = glm::vec3(0.0f);
}

void nex::Mesh::finalize()
{
	if (mVertexArray) return;

	mVertexArray = std::make_unique<VertexArray>();

	mVertexArray->bind();
	mVertexArray->useBuffer(mVertexBuffer, mLayout);

	mVertexArray->unbind();

	// set 'is loaded' state of this resource.
	setIsLoaded();
}

const AABB& Mesh::getAABB() const
{
	return mBoundingBox;
}

void nex::Mesh::addVertexDataBuffer(std::unique_ptr<GpuBuffer> buffer)
{
	mBuffers.emplace_back(std::move(buffer));
}

void Mesh::setBoundingBox(const AABB& box)
{
	mBoundingBox = box;
}

IndexBuffer* Mesh::getIndexBuffer()
{
	return &mIndexBuffer;
}

const VertexLayout& nex::Mesh::getLayout() const
{
	return mLayout;
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
	return mVertexArray.get();
}

/*
void nex::Mesh::init(VertexBuffer vertexBuffer, VertexLayout layout, IndexBuffer indexBuffer, AABB boundingBox, Topology topology)
{
	mLayout = std::move(layout);
	mIndexBuffer = std::move(indexBuffer);
	mVertexBuffer = std::move(vertexBuffer);
	mBoundingBox = std::move(boundingBox);
	mTopology = topology;
	
	setIsLoaded();
}*/

void Mesh::setIndexBuffer(IndexBuffer buffer)
{
	mIndexBuffer = std::move(buffer);
}

void Mesh::setVertexArray(VertexArray vertexArray)
{
	*mVertexArray = std::move(vertexArray);
}