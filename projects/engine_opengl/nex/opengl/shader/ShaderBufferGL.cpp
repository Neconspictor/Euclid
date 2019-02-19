#include <nex/opengl/shader/ShaderBufferGL.hpp>
#include <nex/shader/ShaderBuffer.hpp>
#include <nex/opengl/opengl.hpp>


nex::ShaderStorageBuffer::ShaderStorageBuffer(unsigned binding, size_t size, ShaderBuffer::UsageHint hint) :
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

nex::ShaderStorageBuffer::~ShaderStorageBuffer()
{
	GLCall(glDeleteBuffers(1, &mRendererID));
}

void nex::ShaderStorageBuffer::bind()
{
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mRendererID));
}

size_t nex::ShaderStorageBuffer::getSize() const
{
	return mSize;
}

nex::ShaderBuffer::UsageHint nex::ShaderStorageBuffer::getUsageHint() const
{
	return mUsageHint;
}

void* nex::ShaderStorageBuffer::map(ShaderBuffer::Access usage)
{
	GLCall(void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, translate(usage)));
	return ptr;
}

void nex::ShaderStorageBuffer::unbind()
{
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, GL_FALSE));
}

void nex::ShaderStorageBuffer::unmap()
{
	GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
}

void nex::ShaderStorageBuffer::update(const void* data, size_t size, size_t offset)
{
	GLCall(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data));
}

void nex::ShaderStorageBuffer::createStore(void* data, size_t size, ShaderBuffer::UsageHint hint)
{
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, translate(hint)));
}

nex::UniformBuffer::UniformBuffer(unsigned int binding, size_t size, ShaderBuffer::UsageHint hint) :
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

nex::UniformBuffer::~UniformBuffer()
{
	if (mRendererID != GL_FALSE)
	{
		GLCall(glDeleteBuffers(1, &mRendererID));
		mRendererID = GL_FALSE;
	}
}

void nex::UniformBuffer::createStore(void* data, size_t size, ShaderBuffer::UsageHint hint)
{
	//TODO use glBufferStorage for improved performance!
	GLCall(glBufferData(GL_UNIFORM_BUFFER, size, data, translate(hint)));
	//GLCall(glNamedBufferStorage(mRendererID, size, data, translate(hint)));
}

void nex::UniformBuffer::bind()
{
	//TODO: Use glBindBufferBase???
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, mRendererID));
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, mBinding, mRendererID));
}

size_t nex::UniformBuffer::getSize() const
{
	return mSize;
}

nex::ShaderBuffer::UsageHint nex::UniformBuffer::getUsageHint() const
{
	return mUsageHint;
}

void* nex::UniformBuffer::map(ShaderBuffer::Access usage)
{
	 GLCall(void* ptr = glMapBuffer(GL_UNIFORM_BUFFER, translate(usage)));
	 return ptr;
}

void nex::UniformBuffer::unmap()
{
	GLCall(glUnmapBuffer(GL_UNIFORM_BUFFER));
}

void nex::UniformBuffer::unbind()
{
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, GL_FALSE));
}

void nex::UniformBuffer::update(const void* data, size_t size, size_t offset)
{
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
}

nex::ShaderBuffer::UsageHintGL nex::translate(nex::ShaderBuffer::UsageHint hint)
{
	using namespace ShaderBuffer;

	static UsageHintGL const table[] =
	{
			DYNAMIC_COPY,
			DYNAMIC_DRAW,
			DYNAMIC_READ,

			STATIC_COPY,
			STATIC_READ,
			STATIC_DRAW,

			STREAM_COPY,
			STREAM_DRAW,
			STREAM_READ,
	};

	static const unsigned size = static_cast<unsigned>(UsageHint::LAST) + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "NeX error: UsageHint and UsageHintGL doesn't match");

	return table[static_cast<unsigned>(hint)];
}

nex::ShaderBuffer::AccessGL nex::translate(nex::ShaderBuffer::Access access)
{
	using namespace ShaderBuffer;
	static AccessGL const table[] =
	{
		READ_ONLY,
		WRITE_ONLY,
		READ_WRITE,
	};

	static const unsigned size = static_cast<unsigned>(Access::LAST) + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "NeX error: Access and AccessGL doesn't match");

	return table[static_cast<unsigned>(access)];
}