#include <nex/buffer/VertexBuffer.hpp>
#include <nex/opengl/buffer/GpuBufferGL.hpp>
#include <nex/opengl/opengl.hpp>

nex::VertexBuffer::VertexBuffer(size_t size, const void* data, UsageHint usage) :
	GpuBuffer((void*)GL_ARRAY_BUFFER, size, data, usage)
{
}

nex::VertexBuffer::VertexBuffer(UsageHint usage) : GpuBuffer((void*)GL_ARRAY_BUFFER, 0, nullptr, usage)
{
}

nex::VertexBuffer::~VertexBuffer() = default;

void nex::VertexBuffer::unbindAny() {
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, GL_FALSE));
}