#include <nex/buffer/IndexBuffer.hpp>
#include <nex/opengl/buffer/GpuBufferGL.hpp>
#include <nex/opengl/opengl.hpp>

nex::IndexBuffer::IndexBuffer(IndexElementType type, size_t count, const void* data, UsageHint usage) : IndexBuffer(usage)
{

	fill(type, count, data, usage);
}

nex::IndexBuffer::IndexBuffer(UsageHint usage) : GpuBuffer((void*)GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, usage)
{

}

nex::IndexBuffer::~IndexBuffer() = default;

void nex::IndexBuffer::fill(IndexElementType type, size_t count, const void* data, UsageHint usage)
{
	mCount = count;
	mType = type;

	auto byteSize = sizeof(GLuint);
	if (mType == IndexElementType::BIT_16) byteSize = sizeof(GLushort);

	resize(mCount * byteSize, data, usage);
}