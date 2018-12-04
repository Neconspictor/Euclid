#include <nex/opengl/texture/SamplerGL.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>

using namespace nex;

SamplerGL::SamplerGL(const SamplerState& state) : Sampler(state), m_samplerID(GL_FALSE)
{
	glGenSamplers(1, &m_samplerID);
	setMinFilter(mState.minFilter);
	setMagFilter(mState.minFilter);
	setAnisotropy(mState.anisotropy);
	setWrapS(mState.wrapS);
	setWrapR(mState.wrapR);
	setWrapT(mState.wrapT);
	setBorderColor(mState.borderColor);
	setMinLOD(mState.minLOD);
	setMaxLOD(mState.maxLOD);
	setLodBias(mState.biasLOD);


	setCompareMode(GL_NONE);
	setCompareFunction(GL_LEQUAL);

	mImpl = this;
}

SamplerGL::~SamplerGL()
{
	if (m_samplerID != GL_FALSE) {
		glDeleteSamplers(1, &m_samplerID);
		m_samplerID = GL_FALSE;
	}
}

GLuint SamplerGL::getID() const
{
	return m_samplerID;
}

const SamplerState& Sampler::getState() const
{
	return mState;
}

void Sampler::setMinFilter(TextureFilter filter)
{
	glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MIN_FILTER, translate(filter));
	mState.minFilter = filter;
}

void Sampler::setMagFilter(TextureFilter filter)
{
	glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAG_FILTER, translate(filter));
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
	glGetSamplerParameterIuiv(m_samplerID, GL_TEXTURE_COMPARE_MODE, &result);
	return result;
}

GLuint SamplerGL::getCompareFuntion() const
{
	GLuint result;
	glGetSamplerParameterIuiv(m_samplerID, GL_TEXTURE_COMPARE_FUNC, &result);
	return result;
}

void Sampler::setAnisotropy(float anisotropy)
{
	GLfloat maxAnisotropy = SamplerGL::getMaxAnisotropicFiltering();
	anisotropy = std::min(anisotropy, maxAnisotropy);

	glSamplerParameterf(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
	glGetSamplerParameterfv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAX_ANISOTROPY, &mState.anisotropy);
}

void Sampler::setCompareMode(GLuint mode)
{
	glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_COMPARE_MODE, mode);
}

void Sampler::setCompareFunction(GLuint compareFunction)
{
	glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_COMPARE_FUNC, compareFunction);
}

void Sampler::setWrapS(TextureUVTechnique wrap)
{
	glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_WRAP_S, translate(wrap));
	mState.wrapS = wrap;
}

void Sampler::setWrapT(TextureUVTechnique wrap)
{
	glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_WRAP_T, translate(wrap));
	mState.wrapT = wrap;
}

void Sampler::setWrapR(TextureUVTechnique wrap)
{
	glSamplerParameteri(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_WRAP_R, translate(wrap));
	mState.wrapR = wrap;
}

void Sampler::setBorderColor(const glm::vec4& color)
{
	glSamplerParameterfv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_BORDER_COLOR, (float*)&color.data);
	glGetSamplerParameterfv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_BORDER_COLOR, (float*)&mState.borderColor);
}

void Sampler::setMinLOD(float lod)
{
	glSamplerParameterf(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MIN_LOD, lod);
	glGetSamplerParameterIiv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MIN_LOD, &mState.minLOD);
}

void Sampler::setMaxLOD(float lod)
{
	glSamplerParameterf(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAX_LOD, lod);
	glGetSamplerParameterIiv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_MAX_LOD, &mState.maxLOD);
}

void Sampler::setLodBias(float bias)
{
	glSamplerParameterf(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_LOD_BIAS, bias);
	glGetSamplerParameterIiv(((SamplerGL*)mImpl)->getID(), GL_TEXTURE_LOD_BIAS, &mState.biasLOD);
}