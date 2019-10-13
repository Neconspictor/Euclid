#include <nex/buffer/GpuBuffer.hpp>
#include <nex/opengl/buffer/GpuBufferGL.hpp>
#include <nex/opengl/opengl.hpp>


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

nex::GpuBuffer::Impl::Impl(GLenum target) : mTarget(target), mRendererID(GL_FALSE)
{
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));
	ASSERT(sizeof(unsigned short) == sizeof(GLshort));

	GLCall(glGenBuffers(1, &mRendererID));
	GLCall(glBindBuffer(mTarget, mRendererID));
}

nex::GpuBuffer::Impl::Impl(nex::GpuBuffer::Impl&& other) noexcept :
	mRendererID(other.mRendererID),
	mTarget(other.mTarget)
{
	other.mRendererID = GL_FALSE;
}

nex::GpuBuffer::Impl& nex::GpuBuffer::Impl::operator=(nex::GpuBuffer::Impl&& o) noexcept
{
	if (this == &o) return *this;

	this->mRendererID = o.mRendererID;
	o.mRendererID = GL_FALSE;

	this->mTarget = o.mTarget;

	return *this;
}

nex::GpuBuffer::Impl::~Impl()
{
	if (mRendererID != GL_FALSE) {
		GLCall(glDeleteBuffers(1, &mRendererID));
		mRendererID = GL_FALSE;
	}
}


nex::GpuBuffer::GpuBuffer(void* internalBufferType, size_t size, const void* data, UsageHint usage) :
	mSize(size),
	mUsageHint(usage),

#pragma warning( push )
#pragma warning( disable : 4311) // warning for pointer truncation from void* to GLenum
#pragma warning( disable : 4302) // warning for  truncation from void* to GLenum
	mImpl(new Impl(reinterpret_cast<GLenum>(internalBufferType)))
#pragma warning( pop ) 
{
	resize(mSize, data, mUsageHint);
}

nex::GpuBuffer::GpuBuffer(GpuBuffer&& other) : 
	mSize(other.mSize),
	mUsageHint(other.mUsageHint),
	mImpl(other.mImpl)

{
	other.mImpl = nullptr;
}

nex::GpuBuffer& nex::GpuBuffer::operator=(GpuBuffer&& o) {

	if (this == &o) return *this;

	this->mSize = o.mSize;
	mUsageHint = o.mUsageHint;
	std::swap(mImpl, o.mImpl);
	return *this;
}


nex::GpuBuffer::~GpuBuffer() {
	if (mImpl) delete mImpl;
	mImpl = nullptr;
};

void nex::GpuBuffer::bind() const
{
	GLCall(glBindBuffer(mImpl->mTarget, mImpl->mRendererID));
}

nex::GpuBuffer::Impl* nex::GpuBuffer::getImpl()
{
	return mImpl;
}

const nex::GpuBuffer::Impl* nex::GpuBuffer::getImpl() const
{
	return mImpl;
}

size_t nex::GpuBuffer::getSize() const
{
	return mSize;
}

nex::GpuBuffer::UsageHint nex::GpuBuffer::getUsageHint() const
{
	return mUsageHint;
}

void* nex::GpuBuffer::map(GpuBuffer::Access usage) const
{
	GLCall(void* ptr = glMapNamedBuffer(mImpl->mRendererID, translate(usage)));
	return ptr;
}

void nex::GpuBuffer::unbind() const
{
	GLCall(glBindBuffer(mImpl->mTarget, GL_FALSE));
}

void nex::GpuBuffer::unmap() const
{
	GLCall(glUnmapNamedBuffer(mImpl->mRendererID));
}

void nex::GpuBuffer::update(size_t size, const void* data, size_t offset)
{
	GLCall(glNamedBufferSubData(mImpl->mRendererID, offset, size, data));
}

void nex::GpuBuffer::syncWithGPU()
{
	//GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
	GLCall(glFinish());
}

void nex::GpuBuffer::resize(size_t size, const void* data, GpuBuffer::UsageHint hint)
{
	//bind();
	mUsageHint = hint;
	mSize = size;
	GLCall(glNamedBufferData(mImpl->mRendererID, size, data, translate(hint)));
}



void nex::ShaderBuffer::bindToTarget() const
{
	bindToTarget(mBinding);
}

void nex::ShaderBuffer::bindToTarget(unsigned binding) const
{
	//bind();
	GLCall(glBindBufferBase(GpuBuffer::mImpl->mTarget, binding, GpuBuffer::mImpl->mRendererID));
}

unsigned nex::ShaderBuffer::getDefaultBinding() const
{
	return mBinding;
}

nex::ShaderBuffer::ShaderBuffer(unsigned int binding, void* internalBufferType, size_t size, const void* data, UsageHint usage) :
	GpuBuffer(internalBufferType, size, data, usage), mBinding(binding)
{

}

nex::ShaderBuffer::~ShaderBuffer() = default;