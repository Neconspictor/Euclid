#include <nex/opengl/buffer/GpuBufferGL.hpp>
#include <nex/opengl/opengl.hpp>

nex::GpuBuffer::Impl::Impl(GLenum target) : mTarget(target), mRendererID(GL_FALSE)
{
}


nex::GpuBuffer::GpuBuffer(void* data, size_t size, void* internalBufferType, UsageHint hint) :
	mSize(size),
	mUsageHint(hint),
	mImpl(std::make_unique<Impl>((GLenum)internalBufferType))
{

	GLCall(glGenBuffers(1, &mImpl->mRendererID));
	GLCall(glBindBuffer(mImpl->mTarget, mImpl->mRendererID));
	resize(nullptr, mSize, mUsageHint);
}

nex::GpuBuffer::~GpuBuffer()
{
	if (mImpl->mRendererID != GL_FALSE)
	{
		GLCall(glDeleteBuffers(1, &mImpl->mRendererID));
		mImpl->mRendererID = GL_FALSE;
	}
}

void nex::GpuBuffer::bind()
{
	GLCall(glBindBuffer(mImpl->mTarget, mImpl->mRendererID));
}

size_t nex::GpuBuffer::getSize() const
{
	return mSize;
}

nex::GpuBuffer::UsageHint nex::GpuBuffer::getUsageHint() const
{
	return mUsageHint;
}

void* nex::GpuBuffer::map(GpuBuffer::Access usage)
{
	GLCall(void* ptr = glMapBuffer(mImpl->mTarget, translate(usage)));
	return ptr;
}

void nex::GpuBuffer::unbind()
{
	GLCall(glBindBuffer(mImpl->mTarget, GL_FALSE));
}

void nex::GpuBuffer::unmap()
{
	GLCall(glUnmapBuffer(mImpl->mTarget));
}

void nex::GpuBuffer::update(const void* data, size_t size, size_t offset)
{
	GLCall(glNamedBufferSubData(mImpl->mRendererID, offset, size, data));
}

void nex::GpuBuffer::syncWithGPU()
{
	//GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
	GLCall(glFinish());
}

void nex::GpuBuffer::resize(const void* data, size_t size, GpuBuffer::UsageHint hint)
{
	GLCall(glNamedBufferData(mImpl->mRendererID, size, data, translate(hint)));
}

nex::UsageHintGL nex::translate(nex::GpuBuffer::UsageHint hint)
{
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

	static const unsigned size = static_cast<unsigned>(GpuBuffer::UsageHint::LAST) + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "NeX error: UsageHint and UsageHintGL doesn't match");

	return table[static_cast<unsigned>(hint)];
}

nex::AccessGL nex::translate(nex::GpuBuffer::Access access)
{
	static AccessGL const table[] =
	{
		READ_ONLY,
		WRITE_ONLY,
		READ_WRITE,
	};

	static const unsigned size = static_cast<unsigned>(GpuBuffer::Access::LAST) + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "NeX error: Access and AccessGL doesn't match");

	return table[static_cast<unsigned>(access)];
}