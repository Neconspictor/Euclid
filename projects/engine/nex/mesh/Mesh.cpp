#include <nex/mesh/Mesh.hpp>
#include "nex/material/Material.hpp"
#include <nex/resource/ResourceLoader.hpp>

using namespace std;
using namespace nex;

Mesh::Mesh() : mTopology(Topology::TRIANGLES)
{
}

void nex::Mesh::finalize()
{
	if (!mVertexArray) {
		mVertexArray = std::make_unique<VertexArray>();

		mVertexArray->bind();
		mVertexArray->init(mLayout);

		mVertexArray->unbind();
	}

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

void Mesh::setBoundingBox(AABB box)
{
	mBoundingBox = std::move(box);
}

void nex::Mesh::setLayout(VertexLayout layout)
{
	mLayout = std::move(layout);
}

IndexBuffer& Mesh::getIndexBuffer()
{
	return mIndexBuffer;
}

const IndexBuffer& nex::Mesh::getIndexBuffer() const
{
	return mIndexBuffer;
}

const VertexLayout& nex::Mesh::getLayout() const
{
	return mLayout;
}

VertexLayout& nex::Mesh::getLayout()
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

VertexArray& Mesh::getVertexArray()
{
	return *mVertexArray;
}

const VertexArray& nex::Mesh::getVertexArray() const
{
	return *mVertexArray;
}

std::vector<std::unique_ptr<GpuBuffer>>& nex::Mesh::getVertexBuffers()
{
	return mBuffers;
}

const std::vector<std::unique_ptr<GpuBuffer>>& nex::Mesh::getVertexBuffers() const
{
	return mBuffers;
}

void Mesh::setIndexBuffer(IndexBuffer buffer)
{
	mIndexBuffer = std::move(buffer);
}

void Mesh::setVertexArray(VertexArray vertexArray)
{
	*mVertexArray = std::move(vertexArray);
}

const std::string& nex::SkinnedMesh::getRigID() const
{
	return mRigSID;
}

void nex::SkinnedMesh::setRigID(const std::string& id)
{
	mRigSID = id;
}