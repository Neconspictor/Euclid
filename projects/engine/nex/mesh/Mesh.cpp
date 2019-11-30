#include <nex/mesh/Mesh.hpp>
#include "nex/material/Material.hpp"
#include <nex/resource/ResourceLoader.hpp>

using namespace std;
using namespace nex;

Mesh::Mesh() : mTopology(Topology::TRIANGLES), mUseIndexBuffer(true), mArrayOffset(0), mVertexCount(0)
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

size_t nex::Mesh::getArrayOffset() const
{
	return mArrayOffset;
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

void nex::Mesh::setUseIndexBuffer(bool use)
{
	mUseIndexBuffer = use;
}

void nex::Mesh::setVertexCount(size_t count)
{
	mVertexCount = count;
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

size_t nex::Mesh::getVertexCount() const
{
	return mVertexCount;
}

bool nex::Mesh::getUseIndexBuffer() const
{
	return mUseIndexBuffer;
}

std::vector<std::unique_ptr<GpuBuffer>>& nex::Mesh::getVertexBuffers()
{
	return mBuffers;
}

const std::vector<std::unique_ptr<GpuBuffer>>& nex::Mesh::getVertexBuffers() const
{
	return mBuffers;
}

void nex::Mesh::setArrayOffset(size_t offset)
{
	mArrayOffset = offset;
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