#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <glm/glm.hpp>
#include <nex/opengl/post_processing/SSAO_GL.hpp>
#include <vector>
#include <nex/opengl/drawing/ModelDrawerGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <random>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <nex/gui/Util.hpp>

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


class SSAO_AO_ShaderGL : public ShaderProgramGL
{
public:

	SSAO_AO_ShaderGL(unsigned int kernelSampleSize) : 
	ShaderProgramGL("post_processing/ssao/fullscreenquad.vert.glsl",
		"post_processing/ssao/ssao_deferred_ao_fs.glsl"),
	kernelSampleSize(kernelSampleSize)
	{

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

class SSAO_Tiled_Blur_ShaderGL : public TransformShaderGL
{
public:

	SSAO_Tiled_Blur_ShaderGL() {

		mProgram = new ShaderProgramGL("post_processing/ssao/ssao_tiled_blur_vs.glsl",
			"post_processing/ssao/ssao_tiled_blur_fs.glsl");

		mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
		mAoTexture = { mProgram->getUniformLocation("ssaoInput"), UniformType::TEXTURE2D, 0 };
	}

	virtual ~SSAO_Tiled_Blur_ShaderGL() = default;

	void setAOTexture(const TextureGL* texture) {
		mProgram->setTexture(mAoTexture.location, texture, mAoTexture.textureUnit);
	}

	void setMVP(const glm::mat4& mat) {
		mProgram->setMat4(mTransform.location, mat);
	}

	void onTransformUpdate(const TransformData& data) override
	{
		setMVP((*data.projection)*(*data.view)*(*data.model));
	}

private:
	UniformTex mAoTexture;
	Uniform mTransform;
};


class SSAO_AO_Display_ShaderGL : public TransformShaderGL
{
public:

	SSAO_AO_Display_ShaderGL() {
		mProgram = new ShaderProgramGL("post_processing/ssao/ssao_ao_display_vs.glsl",
			"post_processing/ssao/ssao_ao_display_fs.glsl");

		mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
		mScreenTexture = {mProgram->getUniformLocation("screenTexture"), UniformType::TEXTURE2D, 0};
	}

	virtual ~SSAO_AO_Display_ShaderGL() = default;

	void setScreenTexture(const TextureGL* texture) {
		mProgram->setTexture(mScreenTexture.location, texture, mScreenTexture.textureUnit);
	}

	void setMVP(const glm::mat4& mat) {
		mProgram->setMat4(mTransform.location, mat);
	}

	void onTransformUpdate(const TransformData& data) override
	{
		setMVP((*data.projection)*(*data.view)*(*data.model));
	}

	//TODO
	/*virtual void update(const MeshGL& mesh, const TransformData& data) {
		mat4 const& projection = *data.projection;
		mat4 const& view = *data.view;
		mat4 const& model = *data.model;

		transform = projection * view * model;
		attributes.setData("transform", &transform);
	}*/

private:
	UniformTex mScreenTexture;
	Uniform mTransform;
};


SSAO_RendertargetGL::SSAO_RendertargetGL(int width, int height, GLuint frameBuffer, GLuint ssaoColorBuffer)
	:
	BaseRenderTargetGL(width, height, frameBuffer),
	ssaoColorBuffer(ssaoColorBuffer)
{
}

SSAO_RendertargetGL::SSAO_RendertargetGL(SSAO_RendertargetGL && o) :
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
	windowWidth(windowWidth),
	windowHeight(windowHeight),
	noiseTileWidth(4),
	aoRenderTarget(windowWidth, windowHeight, GL_FALSE, GL_FALSE),
	tiledBlurRenderTarget(windowWidth, windowHeight, GL_FALSE, GL_FALSE),
	modelDrawer(modelDrawer)

{
	// create random kernel samples
	for (unsigned int i = 0; i < SSAO_SAMPLING_SIZE; ++i) {
		vec3 vec;
		vec.x = randomFloat(-1, 1);
		vec.y = randomFloat(-1, 1);
		vec.z = randomFloat(0, 1);

		if (vec.length() != 0)
			normalize(vec);

		//vec *= randomFloat(0, 1);

		float scale = i / (float)SSAO_SAMPLING_SIZE;
		scale = lerp(0.1f, 1.0f, scale * scale);
		//vec *= scale;

		ssaoKernel[i] = move(vec);
	}

	//create noise texture (random rotation vectors in tangent space)
	for (unsigned int i = 0; i < noiseTileWidth * noiseTileWidth; ++i) {
		vec3 vec;
		vec.x = randomFloat(-1, 1);
		vec.y = randomFloat(-1, 1);
		vec.z = 0.0f; // we rotate on z-axis (tangent space); thus no z-component needed

		noiseTextureValues.emplace_back(move(vec));
	}

	m_shaderData.bias = 0.025f;
	m_shaderData.intensity = 1.0f;
	m_shaderData.radius = 0.25f;



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

	tiledBlurPass = make_unique <SSAO_Tiled_Blur_ShaderGL>();

	aoDisplay = make_unique <SSAO_AO_Display_ShaderGL>();


	SSAO_AO_ShaderGL*  configAO = dynamic_cast<SSAO_AO_ShaderGL*>(aoPass.get());
	configAO->setGNoiseTexture(&noiseTexture);

	for (int i = 0; i < SSAO_SAMPLING_SIZE; ++i)
	{
		m_shaderData.samples[i] = vec4(ssaoKernel[i], 0);
	}

	configAO->setSSAOData(&m_shaderData);
}

TextureGL * SSAO_DeferredGL::getAO_Result()
{
	return aoRenderTarget.getTexture();
}

TextureGL * SSAO_DeferredGL::getBlurredResult()
{
	return tiledBlurRenderTarget.getTexture();
}

TextureGL * SSAO_DeferredGL::getNoiseTexture()
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

void SSAO_DeferredGL::renderAO(TextureGL * gPositions, TextureGL * gNormals, const glm::mat4& projectionGPass )
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
	SSAO_Tiled_Blur_ShaderGL* tiledBlurShader = reinterpret_cast<SSAO_Tiled_Blur_ShaderGL*>(tiledBlurPass.get());
	tiledBlurShader->bind();
	tiledBlurShader->setAOTexture(aoRenderTarget.getTexture());

	glViewport(0, 0, tiledBlurRenderTarget.getWidth(), tiledBlurRenderTarget.getHeight());
	glScissor(0, 0, tiledBlurRenderTarget.getWidth(), tiledBlurRenderTarget.getHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, tiledBlurRenderTarget.getFrameBuffer());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		modelDrawer->draw(&screenSprite, tiledBlurShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO_DeferredGL::displayAOTexture(TextureGL* aoTexture)
{
	SSAO_AO_Display_ShaderGL* aoDisplayShader = reinterpret_cast<SSAO_AO_Display_ShaderGL*>(aoDisplay.get());
	//aoDisplayShader.setScreenTexture(tiledBlurRenderTarget.getTexture());
	TextureGL* aoTextureGL = (TextureGL*)aoTexture;

	aoDisplayShader->bind();
	aoDisplayShader->setScreenTexture(aoTextureGL);
	modelDrawer->draw(&screenSprite, aoDisplayShader);
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


SSAOData* SSAO_DeferredGL::getSSAOData()
{
	return &m_shaderData;
}

void SSAO_DeferredGL::setBias(float bias)
{
	m_shaderData.bias = bias;
}

void SSAO_DeferredGL::setItensity(float itensity)
{
	m_shaderData.intensity = itensity;
}

void SSAO_DeferredGL::setRadius(float radius)
{
	m_shaderData.radius = radius;
}

float SSAO_DeferredGL::randomFloat(float a, float b) {
	uniform_real_distribution<float> dist(a, b);
	random_device device;
	default_random_engine gen(device());
	return dist(gen);
}

float SSAO_DeferredGL::lerp(float a, float b, float f) {
	return a + f * (b - a);
}

SSAO_ConfigurationView::SSAO_ConfigurationView(SSAO_DeferredGL* ssao) : m_ssao(ssao)
{
}

void SSAO_ConfigurationView::drawSelf()
{
	// render configuration properties
	ImGui::PushID(m_id.c_str());
	ImGui::LabelText("", "SSAO:");

	SSAOData* data = m_ssao->getSSAOData();

	ImGui::SliderFloat("bias", &data->bias, 0.0f, 5.0f);
	ImGui::SliderFloat("intensity", &data->intensity, 0.0f, 10.0f);
	ImGui::SliderFloat("radius", &data->radius, 0.0f, 10.0f);

	ImGui::Dummy(ImVec2(0, 20));
	nex::engine::gui::Separator(2.0f);

	ImGui::PopID();
}