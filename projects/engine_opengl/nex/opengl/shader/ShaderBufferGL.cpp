#include <nex/opengl/shader/ShaderBufferGL.hpp>
#include "nex/opengl/renderer/RendererOpenGL.hpp"

nex::ShaderStorageBufferGL::ShaderStorageBufferGL(unsigned binding, size_t size, ShaderBufferGL::UsageHint hint) :
	mRendererID(GL_FALSE),
	mBinding(binding),
	mSize(size),
	mUsageHint(hint)
{
	GLCall(glGenBuffers(1, &mRendererID));
	bind();
	createStore(nullptr, mSize, mUsageHint);
	GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, mBinding, mRendererID));
}

nex::ShaderStorageBufferGL::~ShaderStorageBufferGL()
{
	GLCall(glDeleteBuffers(1, &mRendererID));
}

void nex::ShaderStorageBufferGL::bind()
{
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mRendererID));
}

size_t nex::ShaderStorageBufferGL::getSize() const
{
	return mSize;
}

nex::ShaderBufferGL::UsageHint nex::ShaderStorageBufferGL::getUsageHint() const
{
	return mUsageHint;
}

void* nex::ShaderStorageBufferGL::map(ShaderBufferGL::Access usage)
{
	GLCall(void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, usage));
	return ptr;
}

void nex::ShaderStorageBufferGL::unbind()
{
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, GL_FALSE));
}

void nex::ShaderStorageBufferGL::unmap()
{
	GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
}

void nex::ShaderStorageBufferGL::update(const void* data, size_t size, size_t offset)
{
	GLCall(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data));
}

void nex::ShaderStorageBufferGL::createStore(void* data, size_t size, ShaderBufferGL::UsageHint hint)
{
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, hint));
}

nex::UniformBufferGL::UniformBufferGL(unsigned int binding, size_t size, ShaderBufferGL::UsageHint hint) :
mRendererID(GL_FALSE),
mBinding(binding),
mSize(size),
mUsageHint(hint)
{
	GLCall(glGenBuffers(1, &mRendererID));
	bind();
	createStore(nullptr, mSize, mUsageHint);
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, binding, mRendererID));
}

nex::UniformBufferGL::~UniformBufferGL()
{
	GLCall(glDeleteBuffers(1, &mRendererID));
}

void nex::UniformBufferGL::createStore(void* data, size_t size, ShaderBufferGL::UsageHint hint)
{
	GLCall(glBufferData(GL_UNIFORM_BUFFER, size, data, hint));
}

void nex::UniformBufferGL::bind()
{
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, mRendererID));
}

size_t nex::UniformBufferGL::getSize() const
{
	return mSize;
}

nex::ShaderBufferGL::UsageHint nex::UniformBufferGL::getUsageHint() const
{
	return mUsageHint;
}

void* nex::UniformBufferGL::map(ShaderBufferGL::Access usage)
{
	 GLCall(void* ptr = glMapBuffer(GL_UNIFORM_BUFFER, usage));
	 return ptr;
}

void nex::UniformBufferGL::unmap()
{
	GLCall(glUnmapBuffer(GL_UNIFORM_BUFFER));
}

void nex::UniformBufferGL::unbind()
{
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, GL_FALSE));
}

void nex::UniformBufferGL::update(const void* data, size_t size, size_t offset)
{
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
}