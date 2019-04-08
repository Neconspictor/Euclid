#include <nex/opengl/texture/SamplerGL.hpp>
#include <nex/opengl/opengl.hpp>
#include <nex/opengl/RenderBackendGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

using namespace nex;

Sampler::Sampler(const SamplerDesc& samplerState) : mImpl(std::make_unique<SamplerGL>(samplerState))
{
	
	setMinFilter(samplerState.minFilter);
	setMagFilter(samplerState.magFilter);
	setAnisotropy(samplerState.maxAnisotropy);
	setWrapS(samplerState.wrapS);
	setWrapR(samplerState.wrapR);
	setWrapT(samplerState.wrapT);
	setBorderColor(samplerState.borderColor);
	setMinLOD(samplerState.minLOD);
	setMaxLOD(samplerState.maxLOD);
	setLodBias(samplerState.biasLOD);
	useDepthComparison(samplerState.useDepthComparison);
	setCompareFunction(samplerState.compareFunction);
}

Sampler::~Sampler()
{
};


Sampler::Impl::~Impl() = default;

SamplerGL::SamplerGL(const SamplerDesc& state) :  m_samplerID(GL_FALSE), mState(state)
{
	GLCall(glGenSamplers(1, &m_samplerID));
}

SamplerGL::~SamplerGL()
{
	if (m_samplerID != GL_FALSE) {
		GLCall(glDeleteSamplers(1, &m_samplerID));
		m_samplerID = GL_FALSE;
	}
}

GLuint SamplerGL::getID() const
{
	return m_samplerID;
}

void Sampler::bind(unsigned textureBindingSlot)
{
	GLCall(glBindSampler(textureBindingSlot, ((SamplerGL*)mImpl.get())->getID()));
}

const SamplerDesc& Sampler::getState() const
{
	return ((SamplerGL*)mImpl.get())->mState;
}

void Sampler::setMinFilter(TextureFilter filter)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	GLCall(glSamplerParameteri(impl->getID(), GL_TEXTURE_MIN_FILTER, (GLenum)translate(filter)));
	impl->mState.minFilter = filter;
}

void Sampler::setMagFilter(TextureFilter filter)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	GLCall(glSamplerParameteri(((SamplerGL*)mImpl.get())->getID(), GL_TEXTURE_MAG_FILTER, (GLenum)translate(filter)));
	impl->mState.magFilter = filter;
}

GLfloat SamplerGL::getMaxAnisotropicFiltering()
{
	GLfloat maxAnisotropy;
	GLCall(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy));
	return maxAnisotropy;
}

GLuint SamplerGL::getCompareMode() const
{
	GLuint result;
	GLCall(glGetSamplerParameterIuiv(m_samplerID, GL_TEXTURE_COMPARE_MODE, &result));
	return result;
}

GLuint SamplerGL::getCompareFuntion() const
{
	GLuint result;
	GLCall(glGetSamplerParameterIuiv(m_samplerID, GL_TEXTURE_COMPARE_FUNC, &result));
	return result;
}

void Sampler::setAnisotropy(float anisotropy)
{
	GLfloat maxAnisotropy = SamplerGL::getMaxAnisotropicFiltering();
	anisotropy = std::min(anisotropy, maxAnisotropy);

	auto* impl = ((SamplerGL*)mImpl.get());

	GLCall(glSamplerParameterf(impl->getID(), GL_TEXTURE_MAX_ANISOTROPY, anisotropy));
	GLCall(glGetSamplerParameterfv(impl->getID(), GL_TEXTURE_MAX_ANISOTROPY, &impl->mState.maxAnisotropy));
}

void Sampler::useDepthComparison(bool use)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	impl->mState.useDepthComparison = use;
	const GLuint translated = impl->mState.useDepthComparison ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE;
	GLCall(glSamplerParameteri(impl->getID(), GL_TEXTURE_COMPARE_MODE, translated));
}

void Sampler::setCompareFunction(CompareFunction compareFunction)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	impl->mState.compareFunction = compareFunction;
	GLCall(glSamplerParameteri(impl->getID(), GL_TEXTURE_COMPARE_FUNC, (GLenum)translate(impl->mState.compareFunction)));
}

void Sampler::setWrapS(TextureUVTechnique wrap)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	GLCall(glSamplerParameteri(impl->getID(), GL_TEXTURE_WRAP_S, (GLenum)translate(wrap)));
	impl->mState.wrapS = wrap;
}

void Sampler::setWrapT(TextureUVTechnique wrap)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	GLCall(glSamplerParameteri(impl->getID(), GL_TEXTURE_WRAP_T, (GLenum)translate(wrap)));
	impl->mState.wrapT = wrap;
}

void Sampler::setWrapR(TextureUVTechnique wrap)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	GLCall(glSamplerParameteri(impl->getID(), GL_TEXTURE_WRAP_R, (GLenum)translate(wrap)));
	impl->mState.wrapR = wrap;
}

void Sampler::setBorderColor(const glm::vec4& color)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	GLCall(glSamplerParameterfv(impl->getID(), GL_TEXTURE_BORDER_COLOR, (float*)&color.data));
	GLCall(glGetSamplerParameterfv(impl->getID(), GL_TEXTURE_BORDER_COLOR, (float*)&impl->mState.borderColor));
}

void Sampler::setMinLOD(float lod)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	GLCall(glSamplerParameterf(impl->getID(), GL_TEXTURE_MIN_LOD, lod));
	GLCall(glGetSamplerParameterIiv(impl->getID(), GL_TEXTURE_MIN_LOD, &impl->mState.minLOD));
}

void Sampler::setMaxLOD(float lod)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	GLCall(glSamplerParameterf(impl->getID(), GL_TEXTURE_MAX_LOD, lod));
	GLCall(glGetSamplerParameterIiv(impl->getID(), GL_TEXTURE_MAX_LOD, &impl->mState.maxLOD));
}

void Sampler::setLodBias(float bias)
{
	auto* impl = ((SamplerGL*)mImpl.get());
	GLCall(glSamplerParameterf(impl->getID(), GL_TEXTURE_LOD_BIAS, bias));
	GLCall(glGetSamplerParameterfv(impl->getID(), GL_TEXTURE_LOD_BIAS, &impl->mState.biasLOD));
}

void Sampler::unbind(unsigned textureBindingSlot)
{
	GLCall(glBindSampler(textureBindingSlot, GL_FALSE));
}