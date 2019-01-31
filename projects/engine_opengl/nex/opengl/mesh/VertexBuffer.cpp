#include "VertexBuffer.hpp"
#include "nex/opengl/renderer/RendererOpenGL.hpp"

namespace nex
{
	VertexBuffer::VertexBuffer(const void* data, size_t size) : VertexBuffer()
	{
		fill(data, size);
	}

	VertexBuffer::VertexBuffer() : mRendererID(GL_FALSE)
	{
		GLCall(glGenBuffers(1, &mRendererID));
	}

	VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept :
		mRendererID(other.mRendererID)
	{
		other.mRendererID = GL_FALSE;
	}

	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& o) noexcept
	{
		if (this == &o) return *this;

		this->mRendererID = o.mRendererID;
		o.mRendererID = GL_FALSE;

		return *this;
	}

	VertexBuffer::~VertexBuffer()
	{
		if (mRendererID != GL_FALSE)
		{
			GLCall(glDeleteBuffers(1, &mRendererID));
		}
	}

	void VertexBuffer::bind() const
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, mRendererID));
	}

	void VertexBuffer::unbind() const
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	void VertexBuffer::fill(const void* data, size_t size)
	{
		bind();
		GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	}
}
