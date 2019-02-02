#include <nex/mesh/IndexBuffer.hpp>
#include <nex/opengl/opengl.hpp>

namespace nex
{

	IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : IndexBuffer()
	{

		fill(data, count);
	}

	IndexBuffer::IndexBuffer() : mRendererID(GL_FALSE)
	{
		ASSERT(sizeof(unsigned int) == sizeof(GLuint));
		ASSERT(sizeof(unsigned short) == sizeof(GLshort));
		GLCall(glGenBuffers(1, &mRendererID));
	}

	void IndexBuffer::fill(const unsigned int* data, unsigned int count)
	{
		bind();
		mCount = count;
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mCount * sizeof(GLuint), data, GL_STATIC_DRAW));
		mType = IndexElementType::BIT_32;
	}

	void IndexBuffer::fill(const unsigned short* data, unsigned short count)
	{
		bind();
		mCount = count;
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mCount * sizeof(GLshort), data, GL_STATIC_DRAW));
		mType = IndexElementType::BIT_16;
	}

	IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept :
		mRendererID(other.mRendererID),
		mCount(other.mCount)
	{
		other.mRendererID = GL_FALSE;
	}

	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& o) noexcept
	{
		if (this == &o) return *this;

		this->mRendererID = o.mRendererID;
		o.mRendererID = GL_FALSE;

		this->mCount = o.mCount;

		return *this;
	}

	IndexBuffer::~IndexBuffer()
	{
		if (mRendererID != GL_FALSE)
		{
			GLCall(glDeleteBuffers(1, &mRendererID));
		}
	}

	void IndexBuffer::bind() const
	{
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID));
	}

	void IndexBuffer::unbind() const
	{
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}
}