#include <nex/mesh/VertexBuffer.hpp>
#include <nex/opengl/opengl.hpp>
#include "nex/opengl/shader/ShaderBufferGL.hpp"

namespace nex
{
	VertexBuffer::VertexBuffer(const void* data, size_t size) : VertexBuffer()
	{
		fill(data, size);
	}

	VertexBuffer::VertexBuffer() : mRendererID(GL_FALSE), mSize(0)
	{
		GLCall(glGenBuffers(1, &mRendererID));
	}

	VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept :
		mRendererID(other.mRendererID), mSize(other.mSize)
	{
		other.mRendererID = GL_FALSE;
	}

	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& o) noexcept
	{
		if (this == &o) return *this;

		this->mRendererID = o.mRendererID;
		this->mSize = o.mSize;
		o.mRendererID = GL_FALSE;

		return *this;
	}

	VertexBuffer::~VertexBuffer()
	{
		if (mRendererID != GL_FALSE)
		{
			GLCall(glDeleteBuffers(1, &mRendererID));
			mRendererID = GL_FALSE;
		}
	}

	void VertexBuffer::bind()
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, mRendererID));
	}

	void VertexBuffer::unbind()
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	void VertexBuffer::fill(const void* data, size_t size, ShaderBuffer::UsageHint usage)
	{
		bind();
		GLCall(glNamedBufferData(mRendererID, size, data, translate(usage)));
		mSize = size;
	}

	void* VertexBuffer::map(ShaderBuffer::UsageHint usage)
	{
		GLCall(glMapNamedBuffer(mRendererID, translate(usage)));
		return nullptr;
	}

	void VertexBuffer::unmap()
	{
		GLCall(glUnmapNamedBuffer(mRendererID));
	}

	size_t VertexBuffer::size() const
	{
		return mSize;
	}
}
