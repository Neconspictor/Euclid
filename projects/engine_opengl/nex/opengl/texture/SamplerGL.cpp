#include <nex/opengl/texture/SamplerGL.hpp>
#include "nex/opengl/opengl.hpp"

using namespace nex;

SamplerGL::SamplerGL(const SamplerState& state) : Sampler(state, this), m_samplerID(GL_FALSE)
{
	GLCall(glGenSamplers(1, &m_samplerID));

	setMinFilter(mState.minFilter);
	setMagFilter(mState.magFilter);
	setAnisotropy(mState.anisotropy);
	setWrapS(mState.wrapS);
	setWrapR(mState.wrapR);
	setWrapT(mState.wrapT);
	setBorderColor(mState.borderColor);
	setMinLOD(mState.minLOD);
	setMaxLOD(mState.maxLOD);
	setLodBias(mState.biasLOD);
	useDepthComparison(state.useDepthComparison);
	setCompareFunction(state.compareFunction);

	mImpl = this;
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

Sampler* Sampler::create(const SamplerState& samplerState)
{
	return new SamplerGL(samplerState);
}

void Sampler::bind(unsigned textureBindingSlot)
{
	GLCall(glBindSampler(textureBindingSlot, ((SamplerGL*)mImpl)->getID()));
}

const SamplerState& Sampler::getState() const
{
	return mState;
}

void Sampler::setMinFilter(TextureFilter filter)
{
	GLCall(glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MIN_FILTER, translate(filter)));
	mState.minFilter = filter;
}

void Sampler::setMagFilter(TextureFilter filter)
{
	GLCall(glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAG_FILTER, translate(filter)));
	mState.magFilter = filter;
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

	GLCall(glSamplerParameterf(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAX_ANISOTROPY, anisotropy));
	GLCall(glGetSamplerParameterfv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAX_ANISOTROPY, &mState.anisotropy));
}

void Sampler::useDepthComparison(bool use)
{
	mState.useDepthComparison = use;
	const GLuint translated = mState.useDepthComparison ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE;
	GLCall(glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_COMPARE_MODE, translated));
}

void Sampler::setCompareFunction(DepthComparison compareFunction)
{
	mState.compareFunction = compareFunction;
	GLCall(glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_COMPARE_FUNC, translate(mState.compareFunction)));
}

void Sampler::setWrapS(TextureUVTechnique wrap)
{
	GLCall(glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_WRAP_S, translate(wrap)));
	mState.wrapS = wrap;
}

void Sampler::setWrapT(TextureUVTechnique wrap)
{
	GLCall(glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_WRAP_T, translate(wrap)));
	mState.wrapT = wrap;
}

void Sampler::setWrapR(TextureUVTechnique wrap)
{
	GLCall(glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_WRAP_R, translate(wrap)));
	mState.wrapR = wrap;
}

void Sampler::setBorderColor(const glm::vec4& color)
{
	GLCall(glSamplerParameterfv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_BORDER_COLOR, (float*)&color.data));
	GLCall(glGetSamplerParameterfv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_BORDER_COLOR, (float*)&mState.borderColor));
}

void Sampler::setMinLOD(float lod)
{
	GLCall(glSamplerParameterf(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MIN_LOD, lod));
	GLCall(glGetSamplerParameterIiv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MIN_LOD, &mState.minLOD));
}

void Sampler::setMaxLOD(float lod)
{
	GLCall(glSamplerParameterf(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAX_LOD, lod));
	GLCall(glGetSamplerParameterIiv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAX_LOD, &mState.maxLOD));
}

void Sampler::setLodBias(float bias)
{
	GLCall(glSamplerParameterf(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_LOD_BIAS, bias));
	GLCall(glGetSamplerParameterIiv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_LOD_BIAS, &mState.biasLOD));
}

void Sampler::unbind(unsigned textureBindingSlot)
{
	GLCall(glBindSampler(textureBindingSlot, GL_FALSE));
}