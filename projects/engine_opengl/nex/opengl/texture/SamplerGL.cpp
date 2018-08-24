#include <nex/opengl/texture/SamplerGL.hpp>

SamplerGL::SamplerGL() : m_samplerID(GL_FALSE)
{
}

SamplerGL::SamplerGL(GLuint samplerID) : m_samplerID(samplerID)
{
}

SamplerGL::SamplerGL(SamplerGL&& o)
{
	*this = std::move(o);
}

SamplerGL& SamplerGL::operator=(SamplerGL&& o)
{
	if (this == &o) return *this;
	std::swap(m_samplerID, o.m_samplerID);
	return *this;
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

void SamplerGL::setID(GLuint sampler)
{
	m_samplerID = sampler;
}