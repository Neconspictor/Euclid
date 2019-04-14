#include <nex/opengl/texture/SamplerGL.hpp>
#include <nex/opengl/opengl.hpp>
#include <nex/opengl/RenderBackendGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

using namespace nex;

Sampler::Sampler(const SamplerDesc& samplerState) : mImpl(std::make_unique<Sampler::Impl>())
{
	setState(samplerState);
}

Sampler::~Sampler() = default;

Sampler::Impl::Impl(Impl&& o) noexcept : mSamplerID(o.mSamplerID),
mState(std::move(o.mState))
{
	o.mSamplerID = GL_FALSE;
}

Sampler::Impl& Sampler::Impl::operator=(Impl&& o) noexcept
{
	if (this == &o) return *this;
	mSamplerID = o.mSamplerID;
	o.mSamplerID = GL_FALSE;
	mState = std::move(o.mState);

	return *this;
}

Sampler::Impl::Impl() :  mSamplerID(GL_FALSE)
{
	GLCall(glGenSamplers(1, &mSamplerID));
}

Sampler::Impl::~Impl()
{
	if (mSamplerID != GL_FALSE) {
		GLCall(glDeleteSamplers(1, &mSamplerID));
		mSamplerID = GL_FALSE;
	}
}

GLuint Sampler::Impl::getID() const
{
	return mSamplerID;
}

void Sampler::bind(unsigned textureBindingSlot) const
{
	GLCall(glBindSampler(textureBindingSlot, mImpl->getID()));
}

const SamplerDesc& Sampler::getState() const
{
	return mImpl->mState;
}

void Sampler::setMinFilter(TextureFilter filter)
{
	GLCall(glSamplerParameteri(mImpl->getID(), GL_TEXTURE_MIN_FILTER, (GLenum)translate(filter)));
	mImpl->mState.minFilter = filter;
}

void Sampler::setMagFilter(TextureFilter filter)
{
	GLCall(glSamplerParameteri(mImpl->getID(), GL_TEXTURE_MAG_FILTER, (GLenum)translate(filter)));
	mImpl->mState.magFilter = filter;
}

GLfloat Sampler::Impl::getMaxAnisotropicFiltering()
{
	GLfloat maxAnisotropy;
	GLCall(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy));
	return maxAnisotropy;
}

GLuint Sampler::Impl::getCompareMode() const
{
	GLuint result;
	GLCall(glGetSamplerParameterIuiv(mSamplerID, GL_TEXTURE_COMPARE_MODE, &result));
	return result;
}

GLuint Sampler::Impl::getCompareFuntion() const
{
	GLuint result;
	GLCall(glGetSamplerParameterIuiv(mSamplerID, GL_TEXTURE_COMPARE_FUNC, &result));
	return result;
}

void Sampler::setAnisotropy(float anisotropy)
{
	GLfloat maxAnisotropy = Sampler::Impl::getMaxAnisotropicFiltering();
	anisotropy = std::min(anisotropy, maxAnisotropy);

	GLCall(glSamplerParameterf(mImpl->getID(), GL_TEXTURE_MAX_ANISOTROPY, anisotropy));
	GLCall(glGetSamplerParameterfv(mImpl->getID(), GL_TEXTURE_MAX_ANISOTROPY, &mImpl->mState.maxAnisotropy));
}

void Sampler::useDepthComparison(bool use)
{
	mImpl->mState.useDepthComparison = use;
	const GLuint translated = mImpl->mState.useDepthComparison ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE;
	GLCall(glSamplerParameteri(mImpl->getID(), GL_TEXTURE_COMPARE_MODE, translated));
}

void Sampler::setCompareFunction(CompareFunction compareFunction)
{
	mImpl->mState.compareFunction = compareFunction;
	GLCall(glSamplerParameteri(mImpl->getID(), GL_TEXTURE_COMPARE_FUNC, (GLenum)translate(mImpl->mState.compareFunction)));
}

void Sampler::setWrapS(TextureUVTechnique wrap)
{
	GLCall(glSamplerParameteri(mImpl->getID(), GL_TEXTURE_WRAP_S, (GLenum)translate(wrap)));
	mImpl->mState.wrapS = wrap;
}

void Sampler::setWrapT(TextureUVTechnique wrap)
{
	GLCall(glSamplerParameteri(mImpl->getID(), GL_TEXTURE_WRAP_T, (GLenum)translate(wrap)));
	mImpl->mState.wrapT = wrap;
}

void Sampler::setWrapR(TextureUVTechnique wrap)
{
	GLCall(glSamplerParameteri(mImpl->getID(), GL_TEXTURE_WRAP_R, (GLenum)translate(wrap)));
	mImpl->mState.wrapR = wrap;
}

void Sampler::setBorderColor(const glm::vec4& color)
{
	GLCall(glSamplerParameterfv(mImpl->getID(), GL_TEXTURE_BORDER_COLOR, (float*)&color.data));
	GLCall(glGetSamplerParameterfv(mImpl->getID(), GL_TEXTURE_BORDER_COLOR, (float*)&mImpl->mState.borderColor));
}

void Sampler::setMinLOD(float lod)
{
	GLCall(glSamplerParameterf(mImpl->getID(), GL_TEXTURE_MIN_LOD, lod));
	GLCall(glGetSamplerParameterIiv(mImpl->getID(), GL_TEXTURE_MIN_LOD, &mImpl->mState.minLOD));
}

void Sampler::setMaxLOD(float lod)
{
	GLCall(glSamplerParameterf(mImpl->getID(), GL_TEXTURE_MAX_LOD, lod));
	GLCall(glGetSamplerParameterIiv(mImpl->getID(), GL_TEXTURE_MAX_LOD, &mImpl->mState.maxLOD));
}

void Sampler::setLodBias(float bias)
{
	GLCall(glSamplerParameterf(mImpl->getID(), GL_TEXTURE_LOD_BIAS, bias));
	GLCall(glGetSamplerParameterfv(mImpl->getID(), GL_TEXTURE_LOD_BIAS, &mImpl->mState.biasLOD));
}

void Sampler::setState(const SamplerDesc& desc)
{
	setMinFilter(desc.minFilter);
	setMagFilter(desc.magFilter);
	setAnisotropy(desc.maxAnisotropy);
	setWrapS(desc.wrapS);
	setWrapR(desc.wrapR);
	setWrapT(desc.wrapT);
	setBorderColor(desc.borderColor);
	setMinLOD(desc.minLOD);
	setMaxLOD(desc.maxLOD);
	setLodBias(desc.biasLOD);
	useDepthComparison(desc.useDepthComparison);
	setCompareFunction(desc.compareFunction);
}

void Sampler::unbind(unsigned textureBindingSlot)
{
	GLCall(glBindSampler(textureBindingSlot, GL_FALSE));
}