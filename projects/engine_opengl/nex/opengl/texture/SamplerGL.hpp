#pragma once
#include <nex/texture/Sampler.hpp>
#include <glad/glad.h>

class SamplerGL : public Sampler
{
public:
	explicit SamplerGL();
	SamplerGL(GLuint samplerID);
	SamplerGL(SamplerGL&& o);
	SamplerGL& operator=(SamplerGL&& o);


	SamplerGL(const SamplerGL&) = delete;
	SamplerGL& operator=(const SamplerGL&) = delete;

	virtual ~SamplerGL();

	GLuint getID() const;

	void setID(GLuint sampler);

protected:
	GLuint m_samplerID;
};
