#include <texture/TextureGL.hpp>
#include <shader/ShaderGL.hpp>
#include <texture/Texture.hpp>
#include <glm/glm.hpp>
#include <post_processing/HBAO_GL.hpp>
#include <vector>
#include <drawing/ModelDrawerGL.hpp>
#include <algorithm>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std; 
using namespace glm;
using namespace hbao;

HBAO_DeferredGL::HBAO_DeferredGL(unsigned int windowWidth,
	unsigned int windowHeight, ModelDrawerGL* modelDrawer)
	:
	HBAO_Deferred(windowWidth, windowHeight),
	m_modelDrawer(modelDrawer),
	m_depthLinearRT(nullptr),
	m_aoResultRT(nullptr),
	m_aoBlurredResultRT(nullptr),
	m_tempRT(nullptr)

{

	float numDir = 8; // keep in sync to glsl

	static const int randomSize = 4;
	static const int randomElementsCount = randomSize * randomSize;

	signed short hbaoRandomShort[randomElementsCount * 4];

	for (int i = 0; i < randomElementsCount; i++)
	{
		float Rand1 = randomFloat(0, 1);
		float Rand2 = randomFloat(0, 1);

		// Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
		float Angle = 2.f * M_PI * Rand1 / numDir;
		float x = cosf(Angle);
		float y = sinf(Angle);
		float z = Rand2;
		float w = 0;
#define SCALE ((1<<15))
		hbaoRandomShort[i * 4 + 0] = (signed short)(SCALE* x);
		hbaoRandomShort[i * 4 + 1] = (signed short)(SCALE* y);
		hbaoRandomShort[i * 4 + 2] = (signed short)(SCALE* z);
		hbaoRandomShort[i * 4 + 3] = (signed short)(SCALE* w);
#undef SCALE
	}

	GLuint temp;
	//newTexture(textures.hbao_random);
	glGenTextures(1, &temp);
	m_hbao_random.setTexture(temp);
	glBindTexture(GL_TEXTURE_2D_ARRAY, temp);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16_SNORM, randomSize, randomSize, 1);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, randomSize, randomSize, 1, GL_RGBA, GL_SHORT, hbaoRandomShort);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


	glGenTextures(1, &temp);
	m_hbao_randomview.setTexture(temp);
	glTextureView(temp, GL_TEXTURE_2D, m_hbao_random.getTexture(), GL_RGBA16_SNORM, 0, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glCreateBuffers(1, &m_hbao_ubo);
	glNamedBufferStorage(m_hbao_ubo, sizeof(HBAOData), NULL, GL_DYNAMIC_STORAGE_BIT);


	m_aoDisplay = make_unique<hbao::DisplayTex>();
	m_bilateralBlur = make_unique<hbao::BilateralBlur>();
	m_depthLinearizer = make_unique<hbao::DepthLinearizer>();
	m_hbaoShader = make_unique<hbao::HBAO_Shader>();


	initRenderTargets(windowWidth, windowHeight);

	m_bilateralBlur->setSharpness(40.0f);
	m_hbaoShader->setRamdomView(&m_hbao_randomview);

	glGenVertexArrays(1, &m_defaultVAO);
}

HBAO_DeferredGL::~HBAO_DeferredGL()
{
	if (m_hbao_ubo != GL_FALSE) {
		glDeleteBuffers(1, &m_hbao_ubo);
		m_hbao_ubo = GL_FALSE;
	}
}

Texture * HBAO_DeferredGL::getAO_Result()
{
	return m_aoResultRT->getTexture();
}

Texture * HBAO_DeferredGL::getBlurredResult()
{
	return m_aoBlurredResultRT->getTexture();
}

void HBAO_DeferredGL::onSizeChange(unsigned int newWidth, unsigned int newHeight)
{
	this->windowWidth = newWidth;
	this->windowHeight = newHeight;

	initRenderTargets(windowWidth, windowHeight);
}

void HBAO_DeferredGL::renderAO(Texture * depthTexture, const Projection& projection, bool blur)
{
	TextureGL& depthTextureGL = dynamic_cast<TextureGL&>(*depthTexture);
	unsigned int width = m_aoResultRT->getWidth();
	unsigned int height = m_aoResultRT->getHeight();

	prepareHbaoData(projection, width, height);


	glBindVertexArray(m_defaultVAO);
	glViewport(0, 0, m_aoResultRT->getWidth(), m_aoResultRT->getHeight());
	glScissor(0, 0, m_aoResultRT->getWidth(), m_aoResultRT->getHeight());
	
		drawLinearDepth(&depthTextureGL, projection);
	
		// draw hbao to hbao render target
	glBindFramebuffer(GL_FRAMEBUFFER, m_aoResultRT->getFrameBuffer());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		m_hbaoShader->setHbaoData(move(m_hbaoDataSource));
		m_hbaoShader->setLinearDepth(m_depthLinearRT->getTexture());
		m_hbaoShader->setRamdomView(&m_hbao_randomview);
		m_hbaoShader->draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (blur) {
		// clear color/depth/stencil for all involved render targets
		glBindFramebuffer(GL_FRAMEBUFFER, m_aoBlurredResultRT->getFrameBuffer());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, m_tempRT->getFrameBuffer());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// setup bilaterial blur and draw
		m_bilateralBlur->setLinearDepth(m_depthLinearRT->getTexture());
		m_bilateralBlur->setSourceTexture(m_aoResultRT->getTexture(), width, height);
		m_bilateralBlur->draw(m_tempRT.get(), m_aoBlurredResultRT.get());
	}

	glBindVertexArray(0);
}

void HBAO_DeferredGL::displayAOTexture()
{
	//modelDrawer->draw(&screenSprite, *aoDisplay);
	glBindVertexArray(m_defaultVAO);
	m_aoDisplay->setInputTexture(m_aoBlurredResultRT->getTexture());
	m_aoDisplay->draw();
	glBindVertexArray(0);
}


void hbao::HBAO_DeferredGL::prepareHbaoData(const Projection & projection, int width, int height)
{
	// projection
	const float* P = glm::value_ptr(projection.matrix);

	glm::vec4 projInfoPerspective = {
		2.0f / (P[4 * 0 + 0]),       // (x) * (R - L)/N
		2.0f / (P[4 * 1 + 1]),       // (y) * (T - B)/N
		-(1.0f - P[4 * 2 + 0]) / P[4 * 0 + 0], // L/N
		-(1.0f + P[4 * 2 + 1]) / P[4 * 1 + 1], // B/N
	};

	glm::vec4 projInfoOrtho = {
		2.0f / (P[4 * 0 + 0]),      // ((x) * R - L)
		2.0f / (P[4 * 1 + 1]),      // ((y) * T - B)
		-(1.0f + P[4 * 3 + 0]) / P[4 * 0 + 0], // L
		-(1.0f - P[4 * 3 + 1]) / P[4 * 1 + 1], // B
	};

	int useOrtho = projection.perspective ? 0 : 1;
	m_hbaoDataSource.projOrtho = useOrtho;
	m_hbaoDataSource.projInfo = useOrtho ? projInfoOrtho : projInfoPerspective;

	float projScale;
	if (useOrtho) {
		projScale = float(height) / (projInfoOrtho[1]);
	}
	else {
		projScale = float(height) / (tanf(projection.fov * 0.5f) * 2.0f);
	}

	// radius
	float meters2viewspace = 1.0f;
	float radius = 2.0f;
	float intensity = 1.5f;
	float bias = 0.1f;

	float R = radius * meters2viewspace;
	m_hbaoDataSource.R2 = R * R;
	m_hbaoDataSource.NegInvR2 = -1.0f / m_hbaoDataSource.R2;
	m_hbaoDataSource.RadiusToScreen = R * 0.5f * projScale;

	// ao
	m_hbaoDataSource.PowExponent = std::max(intensity, 0.0f);
	m_hbaoDataSource.NDotVBias = std::min(std::max(0.0f, bias), 1.0f);
	m_hbaoDataSource.AOMultiplier = 1.0f / (1.0f - m_hbaoDataSource.NDotVBias);

	// resolution
	int quarterWidth = ((width + 3) / 4);
	int quarterHeight = ((height + 3) / 4);

	m_hbaoDataSource.InvQuarterResolution = vec2(1.0f / float(quarterWidth), 1.0f / float(quarterHeight));
	m_hbaoDataSource.InvFullResolution = vec2(1.0f / float(width), 1.0f / float(height));
}

void hbao::HBAO_DeferredGL::drawLinearDepth(TextureGL* depthTexture, const Projection & projection)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_depthLinearRT->getFrameBuffer());
	m_depthLinearizer->setProjection(&projection);
	m_depthLinearizer->setInputTexture(depthTexture);
	m_depthLinearizer->draw();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void hbao::HBAO_DeferredGL::initRenderTargets(unsigned int width, unsigned int height)
{
	//at first release gpu memory before acquiring new memory
	m_depthLinearRT = nullptr;
	m_aoResultRT = nullptr;
	m_aoBlurredResultRT = nullptr;
	m_tempRT = nullptr;

	// m_depthLinearRT
	GLuint depthLinearTex;
	GLuint depthLinearFBO;
	glGenTextures(1, &depthLinearTex);
	glBindTexture(GL_TEXTURE_2D, depthLinearTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, windowWidth, windowHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &depthLinearFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthLinearFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depthLinearTex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_depthLinearRT = make_unique<OneTextureRenderTarget>(depthLinearFBO, 
		TextureGL(depthLinearTex), 
		width, 
		height);


	// m_aoResultRT
	GLuint aoResultTex;
	GLuint aoResultFBO;
	GLenum formatAO = GL_R8;
	GLint swizzle[4] = { GL_RED,GL_RED,GL_RED,GL_RED };

	glGenTextures(1, &aoResultTex);
	glBindTexture(GL_TEXTURE_2D, aoResultTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, formatAO, width, height);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &aoResultFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, aoResultFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, aoResultTex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_aoResultRT = make_unique<OneTextureRenderTarget>(aoResultFBO,
		TextureGL(aoResultTex),
		width,
		height);


	// m_aoBlurredResultRT
	GLuint aoBlurredResultTex;
	GLuint aoBlurredResultFBO;

	glGenTextures(1, &aoBlurredResultTex);
	glBindTexture(GL_TEXTURE_2D, aoBlurredResultTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, formatAO, width, height);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &aoBlurredResultFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, aoBlurredResultFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, aoBlurredResultTex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_aoBlurredResultRT = make_unique<OneTextureRenderTarget>(aoBlurredResultFBO,
		TextureGL(aoBlurredResultTex),
		width,
		height);


	// m_tempRT
	GLuint tempTex;
	GLuint tempFBO;

	glGenTextures(1, &tempTex);
	glBindTexture(GL_TEXTURE_2D, tempTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, formatAO, width, height);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &tempFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tempTex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	m_tempRT = make_unique<OneTextureRenderTarget>(tempFBO,
		TextureGL(tempTex),
		width,
		height);
}

hbao::BilateralBlur::BilateralBlur() :
	ShaderGL("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/bilateralblur.frag.glsl"),
	m_linearDepth(nullptr),
	m_source(nullptr),
	m_sharpness(0),
	m_textureHeight(0),
	m_textureWidth(0)
{
}

void hbao::BilateralBlur::setLinearDepth(TextureGL * linearDepth)
{
	m_linearDepth = linearDepth;
}

void hbao::BilateralBlur::setSharpness(float sharpness)
{
	m_sharpness = sharpness;
}

void hbao::BilateralBlur::setSourceTexture(TextureGL * source, unsigned int textureWidth, unsigned int textureHeight)
{
	m_source = source;
	m_textureHeight = textureHeight;
	m_textureWidth = textureWidth;
}

void hbao::BilateralBlur::draw(const Mesh & mesh)
{
	throw std::runtime_error("hbao::BilateralBlur::draw(): Function is not supported!");
}

void hbao::BilateralBlur::draw(BaseRenderTargetGL * temp, BaseRenderTargetGL* result)
{
	glBindFramebuffer(GL_FRAMEBUFFER, temp->getFrameBuffer());
	glUseProgram(programID);

	glBindTextureUnit(0, m_source->getTexture());
	glBindTextureUnit(1, m_linearDepth->getTexture());

	glUniform1f(0, m_sharpness);

	// blur horizontal
	glUniform2f(1, 1.0f / float(m_textureWidth), 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// blur vertically
	glBindFramebuffer(GL_FRAMEBUFFER, result->getFrameBuffer());
	glBindTextureUnit(0, temp->getFrameBuffer());
	glUniform2f(1, 0, 1.0f / float(m_textureHeight));
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glUseProgram(0);
}

hbao::DepthLinearizer::DepthLinearizer() : 
	ShaderGL("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/depthlinearize.frag.glsl"),
	m_input(nullptr),
	m_projection(nullptr)
{
}

void hbao::DepthLinearizer::draw(const Mesh & mesh)
{
	throw std::runtime_error("hbao::DepthLinearizer::draw(): Function is not supported!");
}

void hbao::DepthLinearizer::draw()
{
	glUseProgram(programID);
	glUniform4f(0, m_projection->nearplane * m_projection->farplane,
		m_projection->nearplane - m_projection->farplane,
		m_projection->farplane,
		m_projection->perspective ? 1.0f : 0.0f);

	glBindTextureUnit(0, m_input->getTexture());
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);
}

void hbao::DepthLinearizer::setInputTexture(TextureGL * input)
{
	m_input = input;
}

void hbao::DepthLinearizer::setProjection(const Projection* projection)
{
	m_projection = projection;
}

hbao::DisplayTex::DisplayTex() : 
	ShaderGL("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/displaytex.frag.glsl"),
	m_input(nullptr)
{
}

void hbao::DisplayTex::draw(const Mesh & mesh)
{
	throw std::runtime_error("hbao::DisplayTex::draw(): Function is not supported!");
}

void hbao::DisplayTex::draw()
{
	glUseProgram(programID);
	glBindTextureUnit(0, m_input->getTexture());
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);
}

void hbao::DisplayTex::setInputTexture(TextureGL * input)
{
	m_input = input;
}

hbao::HBAO_Shader::HBAO_Shader() : 
	ShaderGL("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/hbao.frag.glsl"),
	m_linearDepth(nullptr),
	m_hbao_randomview(nullptr),
	m_hbao_ubo(GL_FALSE)
{
	memset(&m_hbao_data, 0, sizeof(HBAOData));
}

void hbao::HBAO_Shader::draw(const Mesh & mesh)
{
	throw std::runtime_error("hbao::HBAO_Shader::draw(): Function is not supported!");
}

void hbao::HBAO_Shader::draw()
{
	glUseProgram(programID);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_hbao_ubo);
	glNamedBufferSubData(m_hbao_ubo, 0, sizeof(HBAOData), &m_hbao_data);

	glBindTextureUnit(0, m_linearDepth->getTexture());
	glBindTextureUnit(1, m_hbao_randomview->getTexture());
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);
}

void hbao::HBAO_Shader::setHbaoData(HBAOData hbao)
{
	m_hbao_data = move(hbao);
}

void hbao::HBAO_Shader::setLinearDepth(TextureGL * linearDepth)
{
	m_linearDepth = linearDepth;
}

void hbao::HBAO_Shader::setRamdomView(TextureGL * randomView)
{
	m_hbao_randomview = randomView;
}