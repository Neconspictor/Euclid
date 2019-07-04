#include <nex/mesh/Mesh.hpp>
#include "VertexArray.hpp"
#include "IndexBuffer.hpp"
#include "nex/material/Material.hpp"
#include "VertexLayout.hpp"
#include <nex/resource/ResourceLoader.hpp>

using namespace std;
using namespace nex;

void Mesh::cook()
{
	if (mVertexArray) return;

	mVertexArray = std::make_unique<VertexArray>();

	mVertexArray->bind();
	mVertexArray->useBuffer(mVertexBuffer, mLayout);

	mVertexArray->unbind();

	// set 'is loaded' state of this resource.
	setIsLoaded();
}

void nex::Mesh::finalize()
{
	std::cout << "Called mesh finalization function!" << std::endl;
	cook();
}

Mesh::Mesh(VertexBuffer vertexBuffer, VertexLayout layout, IndexBuffer indexBuffer, AABB boundingBox, Topology topology, bool defer) :
mLayout(std::move(layout)),
mIndexBuffer(std::move(indexBuffer)),
mVertexBuffer(std::move(vertexBuffer)),
mBoundingBox(std::move(boundingBox)),
mTopology(topology)
{
	if (!defer)
		cook();
	else {
		ResourceLoader::get()->enqueue([=] {
			return this;
		});
	}

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
	return mVertexArray.get();
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
	*mVertexArray = std::move(vertexArray);
}