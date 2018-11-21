#include <nex/opengl/texture/SamplerGL.hpp>
#include "nex/opengl/renderer/RendererOpenGL.hpp"

SamplerGL::SamplerGL() : m_samplerID(GL_FALSE)
{
	glGenSamplers(1, &m_samplerID);
}

SamplerGL::SamplerGL(SamplerGL&& o) : m_samplerID(o.m_samplerID)
{
	o.m_samplerID = GL_FALSE;
}

SamplerGL& SamplerGL::operator=(SamplerGL&& o)
{
	if (this == &o) return *this;
	m_samplerID = o.m_samplerID;
	o.m_samplerID = GL_FALSE;
	return *this;
}

SamplerGL::~SamplerGL()
{
	if (m_samplerID != GL_FALSE) {
		glDeleteSamplers(1, &m_samplerID);
		m_samplerID = GL_FALSE;
	}
}

GLfloat SamplerGL::getAnisotropy() const
{
	GLfloat anisotropy;
	glGetSamplerParameterfv(m_samplerID, GL_TEXTURE_MAX_ANISOTROPY, &anisotropy);
	return anisotropy;
}

GLuint SamplerGL::getID() const
{
	return m_samplerID;
}

void SamplerGL::setMinFilter(TextureFilter filter)
{
	glSamplerParameteri(m_samplerID, GL_TEXTURE_MIN_FILTER, TextureGL::mapFilter(filter));
}

void SamplerGL::setMagFilter(TextureFilter filter)
{
	glSamplerParameteri(m_samplerID, GL_TEXTURE_MAG_FILTER, TextureGL::mapFilter(filter));
}

GLfloat SamplerGL::getMaxAnisotropicFiltering()
{
	GLfloat maxAnisotropy;
	GLCall(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy));
	return maxAnisotropy;
}

GLint SamplerGL::getMinFilter() const
{
	GLint filter;
	glGetSamplerParameterIiv(m_samplerID, GL_TEXTURE_MIN_FILTER, &filter);
	return filter;
}

GLint SamplerGL::getMagFilter() const
{
	GLint filter;
	glGetSamplerParameterIiv(m_samplerID, GL_TEXTURE_MAG_FILTER, &filter);
	return filter;
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

GLuint SamplerGL::getWrapS() const
{
	GLuint result;
	glGetSamplerParameterIuiv(m_samplerID, GL_TEXTURE_WRAP_S, &result);
	return result;
}

GLuint SamplerGL::getWrapT() const
{
	GLuint result;
	glGetSamplerParameterIuiv(m_samplerID, GL_TEXTURE_WRAP_T, &result);
	return result;
}

GLuint SamplerGL::getWrapR() const
{
	GLuint result;
	glGetSamplerParameterIuiv(m_samplerID, GL_TEXTURE_WRAP_R, &result);
	return result;
}

glm::vec4 SamplerGL::getBorderColor() const
{
	glm::vec4 result;
	glGetSamplerParameterfv(m_samplerID, GL_TEXTURE_BORDER_COLOR, (float*)&result);
	return result;
}

GLint SamplerGL::getMinLOD() const
{
	GLint result;
	glGetSamplerParameterIiv(m_samplerID, GL_TEXTURE_MIN_LOD, &result);
	return result;
}

GLint SamplerGL::getMaxLOD() const
{
	GLint result;
	glGetSamplerParameterIiv(m_samplerID, GL_TEXTURE_MAX_LOD, &result);
	return result;
}

GLint SamplerGL::getLodBias() const
{
	GLint result;
	glGetSamplerParameterIiv(m_samplerID, GL_TEXTURE_LOD_BIAS, &result);
	return result;
}

void SamplerGL::setAnisotropy(float anisotropy)
{
	GLfloat maxAnisotropy = getMaxAnisotropicFiltering();
	anisotropy = std::min(anisotropy, maxAnisotropy);

	glSamplerParameterf(m_samplerID, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
}

void SamplerGL::setCompareMode(GLuint mode)
{
	glSamplerParameteri(m_samplerID, GL_TEXTURE_COMPARE_MODE, mode);
}

void SamplerGL::setCompareFunction(GLuint compareFunction)
{
	glSamplerParameteri(m_samplerID, GL_TEXTURE_COMPARE_FUNC, compareFunction);
}

void SamplerGL::setWrapS(TextureUVTechnique wrap)
{
	glSamplerParameteri(m_samplerID, GL_TEXTURE_WRAP_S, TextureGL::mapUVTechnique(wrap));
}

void SamplerGL::setWrapT(TextureUVTechnique wrap)
{
	glSamplerParameteri(m_samplerID, GL_TEXTURE_WRAP_T, TextureGL::mapUVTechnique(wrap));
}

void SamplerGL::setWrapR(TextureUVTechnique wrap)
{
	glSamplerParameteri(m_samplerID, GL_TEXTURE_WRAP_R, TextureGL::mapUVTechnique(wrap));
}

void SamplerGL::setBorderColor(const glm::vec4& color)
{
	glSamplerParameterfv(m_samplerID, GL_TEXTURE_BORDER_COLOR, (float*)&color.data);
}

void SamplerGL::setMinLOD(float lod)
{
	glSamplerParameterf(m_samplerID, GL_TEXTURE_MIN_LOD, lod);
}

void SamplerGL::setMaxLOD(float lod)
{
	glSamplerParameterf(m_samplerID, GL_TEXTURE_MAX_LOD, lod);
}

void SamplerGL::setLodBias(float bias)
{
	glSamplerParameterf(m_samplerID, GL_TEXTURE_LOD_BIAS, bias);
}