#include <texture/TextureGL.hpp>
#include <shader/ShaderGL.hpp>
#include <post_processing/SSAO_GL.hpp>
#include <texture/Texture.hpp>
#include <glm/glm.hpp>
#include <post_processing/SSAO_GL.hpp>
#include <vector>

using namespace std; 
using namespace glm;


class SSAO_AO_ShaderGL : public ShaderConfigGL
{
public:

	SSAO_AO_ShaderGL(unsigned int kernelSampleSize) : kernelSampleSize(kernelSampleSize) 
	{
		using types = ShaderAttributeType;

		attributes.create(types::MAT4, &transform, "transform", true);
		attributes.create(types::TEXTURE2D, nullptr, "gNormal");
		attributes.create(types::TEXTURE2D, nullptr, "gPosition");
		attributes.create(types::TEXTURE2D, nullptr, "texNoise");

		for (int i = 0; i < kernelSampleSize; ++i) {
			attributes.create(types::VEC3, nullptr, "samples[" + std::to_string(i) + "]");
		}
	}


	virtual ~SSAO_AO_ShaderGL() = default;

	void setGNoiseTexture(TextureGL* texture) {
		attributes.setData("texNoise", texture);
	}

	void setGNormalTexture(TextureGL* texture) {
		attributes.setData("gNormal", texture);
	}

	void setGPositionTexture(TextureGL* texture) {
		attributes.setData("gPosition", texture);
	}

	void setKernelSamples(const vector<vec3>& vec) {
		assert(kernelSampleSize == vec.size());

		const vec3* ptr = vec.data();
		for (unsigned int i = 0; i < vec.size(); ++i)
			attributes.setData("samples[" + std::to_string(i) + "]", &ptr[i]);
	}

	virtual void update(const MeshGL& mesh, const TransformData& data) {
		mat4 const& projection = *data.projection;
		mat4 const& view = *data.view;
		mat4 const& model = *data.model;

		transform = projection * view * model;
		attributes.setData("transform", &transform);
	}
private:
	glm::mat4 transform;
	unsigned int kernelSampleSize;
};

class SSAO_Tiled_Blur_ShaderGL : public ShaderConfigGL
{
public:

	SSAO_Tiled_Blur_ShaderGL() {

	}

	virtual ~SSAO_Tiled_Blur_ShaderGL() = default;

	void setAOTexture(Texture* texture) {

	}

	virtual void update(const MeshGL& mesh, const TransformData& data) {

	}
};


SSAO_RendertargetGL::SSAO_RendertargetGL(int width, int height, GLuint frameBuffer, GLuint ssaoColorBuffer)
	:
	BaseRenderTarget(width, height),
	BaseRenderTargetGL(width, height, frameBuffer),
	ssaoColorBuffer(ssaoColorBuffer)
{
}

SSAO_RendertargetGL::SSAO_RendertargetGL(SSAO_RendertargetGL && o) :
	BaseRenderTarget(move(o)),
	BaseRenderTargetGL(move(o)),
	ssaoColorBuffer(GL_FALSE)
{
	swap(o);
}

SSAO_RendertargetGL & SSAO_RendertargetGL::operator=(SSAO_RendertargetGL && o)
{
	if (this == &o) return *this;
	BaseRenderTargetGL::operator=(move(o)); // call base class move ao
	swap(o);
	return *this;
}

TextureGL * SSAO_RendertargetGL::getTexture()
{
	return &ssaoColorBuffer;
}

void SSAO_RendertargetGL::swap(SSAO_RendertargetGL & o)
{
	std::swap(ssaoColorBuffer, o.ssaoColorBuffer);
}


SSAO_DeferredGL::SSAO_DeferredGL(unsigned int windowWidth,
	unsigned int windowHeight)
	:
	SSAO_Deferred(windowWidth, windowHeight, 64, 4),
	renderTarget(windowWidth, windowHeight, GL_FALSE, GL_FALSE)

{
	unsigned int noiseTextureSource;
	glGenTextures(1, &noiseTextureSource);
	glBindTexture(GL_TEXTURE_2D, noiseTextureSource);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, noiseTileWidth, noiseTileWidth, 0, GL_RGB, GL_FLOAT, &noiseTextureValues[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	noiseTexture.setTexture(noiseTextureSource);
	renderTarget = createSSAO_FBO(windowWidth, windowHeight);

	aoPass = make_unique <ShaderGL>(make_unique<SSAO_AO_ShaderGL>(64), 
		"post_processing/ssao/ssao_deferred_ao_vs.glsl", 
		"post_processing/ssao/ssao_deferred_ao_fs.glsl");

	tiledBlurPass = make_unique <ShaderGL>(make_unique<SSAO_Tiled_Blur_ShaderGL>(), 
		"post_processing/ssao/ssao_tiled_blur_vs.glsl", 
		"post_processing/ssao/ssao_tiled_blur_fs.glsl");


	SSAO_AO_ShaderGL*  configAO = dynamic_cast<SSAO_AO_ShaderGL*>(aoPass->getConfig());
	configAO->setGNoiseTexture(&noiseTexture);
	configAO->setKernelSamples(ssaoKernel);
}

Texture * SSAO_DeferredGL::getSSAO()
{
	return renderTarget.getTexture();
}

Texture * SSAO_DeferredGL::getNoiseTexture()
{
	return &noiseTexture;
}

void SSAO_DeferredGL::onSizeChange(unsigned int newWidth, unsigned int newHeight)
{
	this->windowWidth = newWidth;
	this->windowHeight = newHeight;
	renderTarget = createSSAO_FBO(newWidth, newHeight);
}

SSAO_RendertargetGL SSAO_DeferredGL::createSSAO_FBO(unsigned int width, unsigned int height)
{
	unsigned int ssaoFBO;
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	unsigned int ssaoColorBuffer;
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

	return SSAO_RendertargetGL(width, height, ssaoFBO, ssaoColorBuffer);
}