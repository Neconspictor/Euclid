#include "IndexBuffer.hpp"
#include "nex/opengl/renderer/RendererOpenGL.hpp"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : mCount(count)
{
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));

	GLCall(glGenBuffers(1, &mRendererID));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mCount * sizeof(GLuint), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
	GLCall(glDeleteBuffers(1, &mRendererID));
}

void IndexBuffer::bind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID));
}

void IndexBuffer::unbind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}