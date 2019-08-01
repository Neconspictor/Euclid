#include <nex/buffer/IndexBuffer.hpp>
#include <nex/opengl/opengl.hpp>

nex::IndexBuffer::IndexBuffer(const void* data, size_t count, IndexElementType type) : IndexBuffer()
{

	fill(data, count, type);
}

nex::IndexBuffer::IndexBuffer() : GpuBuffer((void*)GL_ELEMENT_ARRAY_BUFFER, UsageHint::STATIC_DRAW, 0, nullptr)
{

}

void nex::IndexBuffer::fill(const void* data, size_t count, IndexElementType type, UsageHint usage)
{
	mCount = count;
	mType = type;

	auto byteSize = sizeof(GLuint);
	if (mType == IndexElementType::BIT_16) byteSize = sizeof(GLushort);

	resize(data, mCount * byteSize, usage);
}