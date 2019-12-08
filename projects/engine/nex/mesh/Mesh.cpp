#include <nex/mesh/Mesh.hpp>
#include "nex/material/Material.hpp"
#include <nex/resource/ResourceLoader.hpp>
#include <nex/util/Memory.hpp>

using namespace std;
using namespace nex;

Mesh::Mesh() : mTopology(Topology::TRIANGLES), mUseIndexBuffer(false), mArrayOffset(0), mVertexCount(0)
{
}

void nex::Mesh::finalize()
{
	mVertexArray.init();

	// Note: init binds the vertex array!
	mVertexArray.unbind();

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

void Mesh::setBoundingBox(const AABB& box)
{
	mBoundingBox = box;
}

void nex::Mesh::setUseIndexBuffer(bool use)
{
	mUseIndexBuffer = use;
}

void nex::Mesh::setVertexCount(size_t count)
{
	mVertexCount = count;
}

IndexBuffer* Mesh::getIndexBuffer()
{
	return mIndexBuffer.get();
}

const IndexBuffer* nex::Mesh::getIndexBuffer() const
{
	return mIndexBuffer.get();
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
	return mVertexArray;
}

const VertexArray& nex::Mesh::getVertexArray() const
{
	return mVertexArray;
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

void Mesh::setIndexBuffer(IndexBuffer&& buffer)
{
	setUnique(mIndexBuffer, std::move(buffer));
	mUseIndexBuffer = true;
}

void Mesh::setVertexArray(VertexArray&& vertexArray)
{
	mVertexArray = std::move(vertexArray);	
}

const std::string& nex::SkinnedMesh::getRigID() const
{
	return mRigSID;
}

void nex::SkinnedMesh::setRigID(const std::string& id)
{
	mRigSID = id;
}