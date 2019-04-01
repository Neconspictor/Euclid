#include <nex/mesh/IndexBuffer.hpp>
#include <nex/opengl/opengl.hpp>
#include "nex/opengl/shader/ShaderBufferGL.hpp"

namespace nex
{

	IndexBuffer::IndexBuffer(const void* data, unsigned int count, IndexElementType type) : IndexBuffer()
	{

		fill(data, count, type);
	}

	IndexBuffer::IndexBuffer() : mRendererID(GL_FALSE)
	{
		ASSERT(sizeof(unsigned int) == sizeof(GLuint));
		ASSERT(sizeof(unsigned short) == sizeof(GLshort));
		GLCall(glGenBuffers(1, &mRendererID));
	}

	void IndexBuffer::fill(const void* data, size_t count, IndexElementType type, ShaderBuffer::UsageHint usage)
	{
		bind();
		mCount = count;
		mType = type;

		auto byteSize = sizeof(GLuint);
		if (mType == IndexElementType::BIT_16) byteSize = sizeof(GLushort);

		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mCount * byteSize, data, translate(usage)));
	}

	IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept :
		mRendererID(other.mRendererID),
		mCount(other.mCount),
		mType(other.mType)
	{
		other.mRendererID = GL_FALSE;
	}

	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& o) noexcept
	{
		if (this == &o) return *this;

		this->mRendererID = o.mRendererID;
		o.mRendererID = GL_FALSE;

		this->mCount = o.mCount;
		this->mType = o.mType;

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
