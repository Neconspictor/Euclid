#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/opengl/buffer/GpuBufferGL.hpp>


nex::ShaderStorageBuffer::ShaderStorageBuffer(unsigned int binding, size_t size, void* data, UsageHint usage) :
	ShaderBuffer(binding, (void*)GL_SHADER_STORAGE_BUFFER, size, data, usage)
{
}

nex::ShaderStorageBuffer::~ShaderStorageBuffer() = default;


nex::UniformBuffer::UniformBuffer(unsigned int binding, size_t size, void* data, UsageHint usage) :
	ShaderBuffer(binding, (void*)GL_UNIFORM_BUFFER, size, data, usage)
{
}

nex::UniformBuffer::~UniformBuffer() = default;