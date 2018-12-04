#include <nex/opengl/mesh/MeshGL.hpp>
#include "nex/opengl/renderer/RendererOpenGL.hpp"

using namespace std;
using namespace nex;

MeshGL::MeshGL(VertexArray vertexArray, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, std::unique_ptr<Material> material) :
mVertexArray(std::move(vertexArray)),
mVertexBuffer(std::move(vertexBuffer)),
mIndexBuffer(std::move(indexBuffer)),
mMaterial(std::move(material))
{
}

MeshGL::MeshGL(MeshGL&& o) noexcept :
	mVertexArray(std::move(o.mVertexArray)),
	mVertexBuffer(std::move(o.mVertexBuffer)),
	mIndexBuffer(std::move(o.mIndexBuffer)),
	mMaterial(std::move(o.mMaterial))
{
}

/*MeshGL& MeshGL::operator=(MeshGL&& o) noexcept
{
	if (this == &o) return *this;
	
	mVertexArray = move(o.mVertexArray);
	mIndexBuffer = move(o.mIndexBuffer);
	mMaterial = move(o.mMaterial);
	
	return *this;
}*/

const IndexBuffer* MeshGL::getIndexBuffer() const
{
	return &mIndexBuffer;
}

Material* MeshGL::getMaterial() const
{
	return mMaterial.get();
}

const VertexArray* MeshGL::getVertexArray() const
{
	return &mVertexArray;
}

void MeshGL::setMaterial(std::unique_ptr<Material> material)
{
	mMaterial = std::move(material);
}