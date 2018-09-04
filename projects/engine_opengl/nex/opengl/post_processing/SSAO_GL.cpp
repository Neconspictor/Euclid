#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/texture/Texture.hpp>
#include <glm/glm.hpp>
#include <nex/opengl/post_processing/SSAO_GL.hpp>
#include <vector>
#include <nex/opengl/drawing/ModelDrawerGL.hpp>
#include <nex/util/ExceptionHandling.hpp>

using namespace std; 
using namespace glm;

/*class HBAO_Shader : public ShaderGL {
public:

	HBAO_Shader();
	virtual ~HBAO_Shader() = default;

	virtual void draw(const Mesh& mesh) override;
	void draw();
	void setHbaoData(HBAOData hbao);
	void setHbaoUBO(GLuint hbao_ubo);
	void setLinearDepth(TextureGL* linearDepth);
	void setRamdomView(TextureGL* randomView);

private:
	HBAOData m_hbao_data;
	TextureGL* m_hbao_randomview;
	GLuint m_hbao_ubo;
	TextureGL* m_linearDepth;
};*/


class SSAO_AO_ShaderGL : public ShaderGL
{
public:

	SSAO_AO_ShaderGL(unsigned int kernelSampleSize) : 
	ShaderGL("post_processing/ssao/fullscreenquad.vert.glsl",
		"post_processing/ssao/ssao_deferred_ao_fs.glsl"),
	kernelSampleSize(kernelSampleSize)
	{
		using types = ShaderAttributeType;

		//attributes.create(types::MAT4, &transform, "transform", true);
		//attributes.create(types::MAT4, &projection_GPass, "projection_GPass", true);
		//attributes.create(types::TEXTURE2D, nullptr, "gNormal");
		//attributes.create(types::TEXTURE2D, nullptr, "gPosition");
		//attributes.create(types::TEXTURE2D, nullptr, "texNoise");

		/*for (unsigned int i = 0; i < kernelSampleSize; ++i) {
			attributes.create(types::VEC3, nullptr, "samples[" + std::to_string(i) + "]");
		}*/

		glCreateBuffers(1, &m_ssaoUBO);
		glNamedBufferStorage(m_ssaoUBO, sizeof(SSAOData), NULL, GL_DYNAMIC_STORAGE_BIT);

		// create a vao for rendering fullscreen triangles directly with glDrawArrays
		glGenVertexArrays(1, &m_fullscreenTriangleVAO);
	}


	virtual ~SSAO_AO_ShaderGL() {
		if (m_ssaoUBO != GL_FALSE) {
			glDeleteBuffers(1, &m_ssaoUBO);
			m_ssaoUBO = GL_FALSE;
		}

		if (m_fullscreenTriangleVAO != GL_FALSE) {
			glDeleteVertexArrays(1, &m_fullscreenTriangleVAO);
			m_fullscreenTriangleVAO = GL_FALSE;
		}
	}


	void drawCustom() const
	{
		glUseProgram(programID);
		glBindVertexArray(m_fullscreenTriangleVAO);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ssaoUBO);
		glNamedBufferSubData(m_ssaoUBO, 0, 
			4*4 + 4*4*4, // we update only the first 4 floats + the matrix  <projection_GPass>
			m_ssaoData);

		glBindTextureUnit(0, m_gNormal->getTexture());
		glBindTextureUnit(1, m_gPosition->getTexture());
		glBindTextureUnit(2, m_texNoise->getTexture());
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	void setGNoiseTexture(TextureGL* texture) {
		m_texNoise = texture;
	}

	void setGNormalTexture(TextureGL* texture) {
		m_gNormal = texture;
	}

	void setGPositionTexture(TextureGL* texture) {
		m_gPosition = texture;
	}

	void setKernelSamples(const vector<vec3>& vec) {
		assert(kernelSampleSize == vec.size());

		m_samples = &vec;

		/*const vec3* ptr = vec.data();
		for (unsigned int i = 0; i < vec.size(); ++i)
			attributes.setData("samples[" + std::to_string(i) + "]", &ptr[i]);*/
	}

	void setProjectionGPass(glm::mat4 matrix) {
		projection_GPass = move(matrix);
	}

	void setSSAOData(SSAOData* data)
	{
		m_ssaoData = data;

		glUseProgram(programID);
		glBindVertexArray(m_fullscreenTriangleVAO);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ssaoUBO);
		glNamedBufferSubData(m_ssaoUBO, 0, sizeof(SSAOData), m_ssaoData);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	/*virtual void update(const MeshGL& mesh, const TransformData& data) {
		mat4 const& projection = *data.projection;
		mat4 const& view = *data.view;
		mat4 const& model = *data.model;

		transform = projection * view * model;
		attributes.setData("transform", &transform);
	}*/



private:
	glm::mat4 transform;
	glm::mat4 projection_GPass;
	unsigned int kernelSampleSize;
	SSAOData* m_ssaoData;
	GLuint m_ssaoUBO;
	TextureGL* m_texNoise;
	TextureGL* m_gPosition;
	TextureGL* m_gNormal;
	const std::vector<vec3>* m_samples;

	GLuint m_fullscreenTriangleVAO;
};

class SSAO_Tiled_Blur_ShaderGL : public ShaderConfigGL
{
public:

	SSAO_Tiled_Blur_ShaderGL() {
		using types = ShaderAttributeType;

		attributes.create(types::MAT4, &transform, "transform", true);
		attributes.create(types::TEXTURE2D, nullptr, "ssaoInput");
	}

	virtual ~SSAO_Tiled_Blur_ShaderGL() = default;

	void setAOTexture(TextureGL* texture) {
		attributes.setData("ssaoInput", texture);
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
};


class SSAO_AO_Display_ShaderGL : public ShaderConfigGL
{
public:

	SSAO_AO_Display_ShaderGL() {
		using types = ShaderAttributeType;

		attributes.create(types::MAT4, &transform, "transform", true);
		attributes.create(types::TEXTURE2D, nullptr, "screenTexture");
	}

	virtual ~SSAO_AO_Display_ShaderGL() = default;

	void setScreenTexture(TextureGL* texture) {
		attributes.setData("screenTexture", texture);
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
	unsigned int windowHeight, ModelDrawerGL* modelDrawer)
	:
	SSAO_Deferred(windowWidth, windowHeight, 4),
	aoRenderTarget(windowWidth, windowHeight, GL_FALSE, GL_FALSE),
	tiledBlurRenderTarget(windowWidth, windowHeight, GL_FALSE, GL_FALSE),
	modelDrawer(modelDrawer)

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

	aoRenderTarget = createSSAO_FBO(windowWidth, windowHeight);
	tiledBlurRenderTarget = createSSAO_FBO(windowWidth, windowHeight);

	aoPass = make_unique <SSAO_AO_ShaderGL>(32);

	tiledBlurPass = make_unique <ShaderGL>(make_unique<SSAO_Tiled_Blur_ShaderGL>(), 
		"post_processing/ssao/ssao_tiled_blur_vs.glsl", 
		"post_processing/ssao/ssao_tiled_blur_fs.glsl");

	aoDisplay = make_unique <ShaderGL>(make_unique<SSAO_AO_Display_ShaderGL>(),
		"post_processing/ssao/ssao_ao_display_vs.glsl",
		"post_processing/ssao/ssao_ao_display_fs.glsl");


	SSAO_AO_ShaderGL*  configAO = dynamic_cast<SSAO_AO_ShaderGL*>(aoPass.get());
	configAO->setGNoiseTexture(&noiseTexture);

	for (int i = 0; i < SSAO_SAMPLING_SIZE; ++i)
	{
		m_shaderData.samples[i] = vec4(ssaoKernel[i], 0);
	}

	configAO->setSSAOData(&m_shaderData);
}

Texture * SSAO_DeferredGL::getAO_Result()
{
	return aoRenderTarget.getTexture();
}

Texture * SSAO_DeferredGL::getBlurredResult()
{
	return tiledBlurRenderTarget.getTexture();
}

Texture * SSAO_DeferredGL::getNoiseTexture()
{
	return &noiseTexture;
}

void SSAO_DeferredGL::onSizeChange(unsigned int newWidth, unsigned int newHeight)
{
	this->windowWidth = newWidth;
	this->windowHeight = newHeight;
	aoRenderTarget = createSSAO_FBO(newWidth, newHeight);
	tiledBlurRenderTarget = createSSAO_FBO(newWidth, newHeight);
}

void SSAO_DeferredGL::renderAO(Texture * gPositions, Texture * gNormals, const glm::mat4& projectionGPass )
{
	TextureGL& gPositionsGL = dynamic_cast<TextureGL&>(*gPositions);
	TextureGL& gNormalsGL = dynamic_cast<TextureGL&>(*gNormals);
	SSAO_AO_ShaderGL&  aoShader = dynamic_cast<SSAO_AO_ShaderGL&>(*aoPass);

	aoShader.setGNormalTexture(&gNormalsGL);
	aoShader.setGPositionTexture(&gPositionsGL);

	m_shaderData.projection_GPass = projectionGPass;

	glViewport(0, 0, aoRenderTarget.getWidth(), aoRenderTarget.getHeight());
	glScissor(0, 0, aoRenderTarget.getWidth(), aoRenderTarget.getHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, aoRenderTarget.getFrameBuffer());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		aoShader.drawCustom();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO_DeferredGL::blur()
{
	SSAO_Tiled_Blur_ShaderGL& tiledBlurShader = dynamic_cast<SSAO_Tiled_Blur_ShaderGL&>(*tiledBlurPass->getConfig());
	tiledBlurShader.setAOTexture(aoRenderTarget.getTexture());
	glViewport(0, 0, tiledBlurRenderTarget.getWidth(), tiledBlurRenderTarget.getHeight());
	glScissor(0, 0, tiledBlurRenderTarget.getWidth(), tiledBlurRenderTarget.getHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, tiledBlurRenderTarget.getFrameBuffer());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		modelDrawer->draw(&screenSprite, *tiledBlurPass);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO_DeferredGL::displayAOTexture(Texture* aoTexture)
{
	SSAO_AO_Display_ShaderGL& aoDisplayShader = dynamic_cast<SSAO_AO_Display_ShaderGL&>(*aoDisplay->getConfig());
	//aoDisplayShader.setScreenTexture(tiledBlurRenderTarget.getTexture());
	TextureGL* aoTextureGL = (TextureGL*)aoTexture;
	aoDisplayShader.setScreenTexture(aoTextureGL);
	modelDrawer->draw(&screenSprite, *aoDisplay);
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

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw_with_trace(std::runtime_error("SSAO Framebuffer not complete!"));

	return SSAO_RendertargetGL(width, height, ssaoFBO, ssaoColorBuffer);
}