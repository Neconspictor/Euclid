#include <nex/opengl/mesh/MeshGL.hpp>
#include "nex/opengl/renderer/RendererOpenGL.hpp"

using namespace std;
using namespace nex;

MeshGL::MeshGL(VertexArray vertexArray, IndexBuffer indexBuffer, Material* material) :
mVertexArray(std::move(vertexArray)),
mIndexBuffer(std::move(indexBuffer)),
mMaterial(material)
{
}

MeshGL::MeshGL(): mMaterial(nullptr)
{
}

/*MeshGL::MeshGL(MeshGL&& o) noexcept :
	mVertexArray(std::move(o.mVertexArray)),
	mIndexBuffer(std::move(o.mIndexBuffer)),
	mMaterial(o.mMaterial)
{
}*/

/*MeshGL& MeshGL::operator=(MeshGL&& o) noexcept
{
	if (this == &o) return *this;
	
	mVertexArray = move(o.mVertexArray);
	mIndexBuffer = move(o.mIndexBuffer);
	mMaterial = move(o.mMaterial);
	
	return *this;
}*/

IndexBuffer* MeshGL::getIndexBuffer()
{
	return &mIndexBuffer;
}

Material* MeshGL::getMaterial() const
{
	return mMaterial;
}

VertexArray* MeshGL::getVertexArray()
{
	return &mVertexArray;
}

void MeshGL::setIndexBuffer(IndexBuffer buffer)
{
	mIndexBuffer = std::move(buffer);
}

void MeshGL::setVertexArray(VertexArray vertexArray)
{
	mVertexArray = std::move(vertexArray);
}

void MeshGL::setMaterial(Material* material)
{
	mMaterial = material;
}