#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <glm/glm.hpp>
#include <nex/opengl/post_processing/HBAO_GL.hpp>
#include <nex/opengl/drawing/ModelDrawerGL.hpp>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <nex/util/Math.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/gui/ImGUI.hpp>
#include <iostream>
#include <random>
#include <nex/gui/Util.hpp>
#include "nex/opengl/renderer/RendererOpenGL.hpp"

// GCC under MINGW has no support for a real random device!
#if defined(__MINGW32__)  && defined(__GNUC__)
#include <boost/random/random_device.hpp>
#endif


using namespace std; 
using namespace glm;

namespace nex
{

	HBAO_GL::HBAO_GL(unsigned int windowWidth,
		unsigned int windowHeight, ModelDrawerGL* modelDrawer)
		:
		m_blur_sharpness(40.0f),
		m_meters2viewspace(1.0f),
		m_radius(2.0f),
		m_intensity(1.5f),
		m_bias(0.1f),
		windowWidth(windowWidth),
		windowHeight(windowHeight),
		m_depthLinearRT(nullptr),
		m_aoResultRT(nullptr),
		m_tempRT(nullptr),
		m_modelDrawer(modelDrawer),
		m_fullscreenTriangleVAO(GL_FALSE)

	{

		signed short hbaoRandomShort[HBAO_RANDOM_ELEMENTS * 4];

		for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i++)
		{
			float Rand1 = randomFloat(0, 1);
			float Rand2 = randomFloat(0, 1);

			// Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
			static float pi = static_cast<float>(nex::util::PI);
			static float radiantFraction = 2.0f * pi / (float)HBAO_NUM_DIRECTIONS;

			float Angle = Rand1 * radiantFraction;
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
		m_hbao_random = Texture::create();
		TextureGL* randomGL = (TextureGL*)m_hbao_random->getImpl();
		GLCall(glGenTextures(1, &temp));
		randomGL->setTexture(temp);
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D_ARRAY, temp));
		GLCall(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16_SNORM, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, 1));
		GLCall(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, 1, GL_RGBA, GL_SHORT, hbaoRandomShort));
		GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));


		glGenTextures(1, &temp);
		m_hbao_randomview = Texture::create();
		TextureGL* randomViewGL = (TextureGL*)m_hbao_randomview->getImpl();
		randomViewGL->setTexture(temp);
		GLCall(glTextureView(*randomViewGL->getTexture(), GL_TEXTURE_2D, *randomGL->getTexture(), GL_RGBA16_SNORM, 0, 1, 0, 1));
		GLCall(glBindTexture(GL_TEXTURE_2D, *randomViewGL->getTexture()));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));

		GLCall(glCreateBuffers(1, &m_hbao_ubo));
		GLCall(glNamedBufferStorage(m_hbao_ubo, sizeof(HBAOData), NULL, GL_DYNAMIC_STORAGE_BIT));


		m_aoDisplay = std::make_unique<DisplayTex>();
		m_bilateralBlur = std::make_unique<BilateralBlur>();
		m_depthLinearizer = std::make_unique<DepthLinearizer>();
		m_hbaoShader = std::make_unique<HBAO_Shader>();


		initRenderTargets(windowWidth, windowHeight);

		// initialize static shader attributes
		m_hbaoShader->setRamdomView(m_hbao_randomview.get());
		m_hbaoShader->setHbaoUBO(m_hbao_ubo);

		// create a vao for rendering fullscreen triangles directly with glDrawArrays
		GLCall(glGenVertexArrays(1, &m_fullscreenTriangleVAO));
	}

	HBAO_GL::~HBAO_GL()
	{
		if (m_hbao_ubo != GL_FALSE) {
			glDeleteBuffers(1, &m_hbao_ubo);
			m_hbao_ubo = GL_FALSE;
		}

		if (m_fullscreenTriangleVAO != GL_FALSE) {
			glDeleteVertexArrays(1, &m_fullscreenTriangleVAO);
			m_fullscreenTriangleVAO = GL_FALSE;
		}
	}

	Texture * HBAO_GL::getAO_Result()
	{
		return m_aoResultRT->getTexture();
	}

	Texture * HBAO_GL::getBlurredResult()
	{
		return m_aoBlurredResultRT->getTexture();
	}

	void HBAO_GL::onSizeChange(unsigned int newWidth, unsigned int newHeight)
	{
		this->windowWidth = newWidth;
		this->windowHeight = newHeight;

		initRenderTargets(windowWidth, windowHeight);
	}

	void HBAO_GL::renderAO(Texture * depthTexture, const Projection& projection, bool blur)
	{
		unsigned int width = m_aoResultRT->getWidth();
		unsigned int height = m_aoResultRT->getHeight();

		prepareHbaoData(projection, width, height);


		GLCall(glBindVertexArray(m_fullscreenTriangleVAO));
		GLCall(glViewport(0, 0, m_aoResultRT->getWidth(), m_aoResultRT->getHeight()));
		GLCall(glScissor(0, 0, m_aoResultRT->getWidth(), m_aoResultRT->getHeight()));

		drawLinearDepth(depthTexture, projection);

		// draw hbao to hbao render target
		m_aoResultRT->bind();
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

		m_hbaoShader->setHbaoData(move(m_hbaoDataSource));
		m_hbaoShader->setLinearDepth(m_depthLinearRT->getTexture());
		m_hbaoShader->setRamdomView(m_hbao_randomview.get());
		m_hbaoShader->draw();

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		if (blur) {
			// clear color/depth/stencil for all involved render targets
			m_aoBlurredResultRT->bind();
			GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
			m_tempRT->bind();
			GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

			// setup bilaterial blur and draw
			m_bilateralBlur->setLinearDepth(m_depthLinearRT->getTexture());
			m_bilateralBlur->setSourceTexture(m_aoResultRT->getTexture(), width, height);
			m_bilateralBlur->setSharpness(m_blur_sharpness);
			m_bilateralBlur->draw(m_tempRT.get(), m_aoBlurredResultRT.get());
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GLCall(glBindVertexArray(0));
	}

	void HBAO_GL::displayAOTexture(Texture* texture)
	{
		//modelDrawer->draw(&screenSprite, *aoDisplay);
		GLCall(glBindVertexArray(m_fullscreenTriangleVAO));
		m_aoDisplay->setInputTexture(texture);
		m_aoDisplay->draw();
		GLCall(glBindVertexArray(0));
	}


	void HBAO_GL::prepareHbaoData(const Projection & projection, int width, int height)
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

		float R = m_radius * m_meters2viewspace;
		m_hbaoDataSource.R2 = R * R;
		m_hbaoDataSource.NegInvR2 = -1.0f / m_hbaoDataSource.R2;
		m_hbaoDataSource.RadiusToScreen = R * 0.5f * projScale;

		// ao
		m_hbaoDataSource.PowExponent = std::max(m_intensity, 0.0f);
		m_hbaoDataSource.NDotVBias = std::min(std::max(0.0f, m_bias), 1.0f);
		m_hbaoDataSource.AOMultiplier = 1.0f / (1.0f - m_hbaoDataSource.NDotVBias);

		// resolution
		int quarterWidth = ((width + 3) / 4);
		int quarterHeight = ((height + 3) / 4);

		m_hbaoDataSource.InvQuarterResolution = vec2(1.0f / float(quarterWidth), 1.0f / float(quarterHeight));
		m_hbaoDataSource.InvFullResolution = vec2(1.0f / float(width), 1.0f / float(height));
	}

	void HBAO_GL::drawLinearDepth(Texture* depthTexture, const Projection & projection)
	{
		m_depthLinearRT->bind();
		m_depthLinearizer->setProjection(&projection);
		m_depthLinearizer->setInputTexture(depthTexture);
		m_depthLinearizer->draw();
	}

	void HBAO_GL::initRenderTargets(unsigned int width, unsigned int height)
	{
		//at first release gpu memory before acquiring new memory
		m_depthLinearRT = nullptr;
		m_aoResultRT = nullptr;
		m_aoBlurredResultRT = nullptr;
		m_tempRT = nullptr;

		// m_depthLinearRT
		TextureData data;
		data.internalFormat = InternFormat::R32F;
		data.magFilter = TextureFilter::NearestNeighbor;
		data.minFilter = TextureFilter::NearestNeighbor;
		data.wrapR = TextureUVTechnique::ClampToEdge;
		data.wrapS = TextureUVTechnique::ClampToEdge;
		data.wrapT = TextureUVTechnique::ClampToEdge;
		data.pixelDataType = PixelDataType::FLOAT;
		data.generateMipMaps = false;
		data.colorspace = ColorSpace::R;

		m_depthLinearRT.reset(RenderTarget::createSingleSampled(width, height, data, nullptr));

		// m_aoResultRT
		data.internalFormat = InternFormat::R8;
		data.useSwizzle = true;
		data.swizzle = { Channel::RED, Channel::RED, Channel::RED, Channel::RED };
		data.wrapR = TextureUVTechnique::ClampToEdge;
		data.wrapS = TextureUVTechnique::ClampToEdge;
		data.wrapT = TextureUVTechnique::ClampToEdge;
		data.pixelDataType = PixelDataType::UBYTE;
		data.generateMipMaps = false;
		data.colorspace = ColorSpace::R;
		m_aoResultRT.reset(RenderTarget::createSingleSampled(width, height, data, nullptr));

		// m_aoBlurredResultRT
		m_aoBlurredResultRT.reset(RenderTarget::createSingleSampled(width, height, data, nullptr));

		// m_tempRT
		m_tempRT.reset(RenderTarget::createSingleSampled(width, height, data, nullptr));
	}

	BilateralBlur::BilateralBlur() :
		Shader(),
		m_linearDepth(nullptr),
		m_sharpness(0),
		m_textureWidth(0),
		m_textureHeight(0),
		m_source(nullptr)
	{
		mProgram = ShaderProgram::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/bilateralblur.frag.glsl");
	}

	void BilateralBlur::setLinearDepth(Texture * linearDepth)
	{
		m_linearDepth = linearDepth;
	}

	void BilateralBlur::setSharpness(float sharpness)
	{
		m_sharpness = sharpness;
	}

	void BilateralBlur::setSourceTexture(Texture * source, unsigned int textureWidth, unsigned int textureHeight)
	{
		m_source = source;
		m_textureHeight = textureHeight;
		m_textureWidth = textureWidth;
	}

	void BilateralBlur::draw(RenderTarget* temp, RenderTarget* result)
	{
		temp->bind();
		bind();

		TextureGL* sourceGL = (TextureGL*)m_source->getImpl();
		TextureGL* linearDepthGL = (TextureGL*)m_linearDepth->getImpl();

		GLCall(glBindTextureUnit(0, *sourceGL->getTexture()));
		GLCall(glBindTextureUnit(1, *linearDepthGL->getTexture()));

		GLCall(glUniform1f(0, m_sharpness));

		// blur horizontal
		GLCall(glUniform2f(1, 1.0f / (float)m_textureWidth, 0));
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 3));

		// blur vertically
		result->bind();
		TextureGL* tempTexGL = (TextureGL*)temp->getTexture()->getImpl();

		GLCall(glBindTextureUnit(0, *tempTexGL->getTexture()));
		GLCall(glUniform2f(1, 0, 1.0f / (float)m_textureHeight));
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 3));
	}

	DepthLinearizer::DepthLinearizer() :
		Shader(),
		m_input(nullptr),
		m_projection(nullptr)
	{
		mProgram = ShaderProgram::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/depthlinearize.frag.glsl");
	}

	void DepthLinearizer::draw()
	{
		bind();
		GLCall(glUniform4f(0, m_projection->nearplane * m_projection->farplane,
			m_projection->nearplane - m_projection->farplane,
			m_projection->farplane,
			m_projection->perspective ? 1.0f : 0.0f));

		TextureGL* inputGL = (TextureGL*)m_input->getImpl();
		GLuint texture = *inputGL->getTexture();
		GLCall(glBindTextureUnit(0, texture));
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 3));
		GLCall(glUseProgram(0));
	}

	void DepthLinearizer::setInputTexture(Texture * input)
	{
		m_input = input;
	}

	void DepthLinearizer::setProjection(const Projection* projection)
	{
		m_projection = projection;
	}

	DisplayTex::DisplayTex() : Shader(),
		m_input(nullptr)
	{
		mProgram = ShaderProgram::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/displaytex.frag.glsl");
	}

	void DisplayTex::draw()
	{
		bind();
		TextureGL* inputGL = (TextureGL*)m_input->getImpl();
		GLCall(glBindTextureUnit(0, *inputGL->getTexture()));
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 3));
	}

	void DisplayTex::setInputTexture(Texture * input)
	{
		m_input = input;
	}

	HBAO_Shader::HBAO_Shader() :
		Shader(),
		m_linearDepth(nullptr),
		m_hbao_randomview(nullptr),
		m_hbao_ubo(GL_FALSE)
	{
		memset(&m_hbao_data, 0, sizeof(HBAOData));

		mProgram = ShaderProgram::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/hbao.frag.glsl");
	}

	void HBAO_Shader::draw()
	{
		bind();
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_hbao_ubo));
		GLCall(glNamedBufferSubData(m_hbao_ubo, 0, sizeof(HBAOData), &m_hbao_data));

		TextureGL* linearDepthGL = (TextureGL*)m_linearDepth->getImpl();
		TextureGL* randomViewGL = (TextureGL*)m_hbao_randomview->getImpl();

		GLCall(glBindTextureUnit(0, *linearDepthGL->getTexture()));
		GLCall(glBindTextureUnit(1, *randomViewGL->getTexture()));
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 3));
	}

	void HBAO_Shader::setHbaoData(HBAOData hbao)
	{
		m_hbao_data = move(hbao);
	}

	void HBAO_Shader::setHbaoUBO(GLuint hbao_ubo)
	{
		m_hbao_ubo = hbao_ubo;
	}

	void HBAO_Shader::setLinearDepth(Texture * linearDepth)
	{
		m_linearDepth = linearDepth;
	}

	void HBAO_Shader::setRamdomView(Texture * randomView)
	{
		m_hbao_randomview = randomView;
	}


	float HBAO_GL::getBlurSharpness() const
	{
		return m_blur_sharpness;
	}

	void HBAO_GL::setBlurSharpness(float sharpness)
	{
		m_blur_sharpness = sharpness;
	}

	float HBAO_GL::randomFloat(float a, float b)
	{
		// GCC under MINGW has no support for a real random device!
#if defined(__MINGW32__)  && defined(__GNUC__)
	//typedef boost::mt19937 gen_type;
	//long unsigned int seed = std::chrono::steady_clock::now().time_since_epoch().count();
		boost::random::random_device seeder;
		boost::random::mt19937 rng(seeder());
		boost::random::uniform_real_distribution<double> gen(a, b);
		return gen(rng);
#else
		uniform_real_distribution<double> dist(a, b);
		random_device device;
		mt19937 gen(device());
		return dist(gen);

#endif
	}

	float nex::HBAO_GL::lerp(float a, float b, float f) {
		return a + f * (b - a);
	}

	class Test {
	public:
		void operator()() const
		{
			if (ImGui::MenuItem("operator()")) {
				std::cout << "called Test()" << std::endl;
			}
		}
	};

	HBAO_ConfigurationView::HBAO_ConfigurationView(HBAO_GL * hbao) : m_hbao(hbao)
	{
		m_isVisible = true;
	}

	void HBAO_ConfigurationView::drawSelf()
	{
		// render configuration properties
		ImGui::PushID(m_id.c_str());
		ImGui::LabelText("", "HBAO:");
		ImGui::SliderFloat("bias", &m_hbao->m_bias, 0.0f, 5.0f);
		ImGui::SliderFloat("blur sharpness", &m_hbao->m_blur_sharpness, 0.0f, 1000.0f);
		ImGui::SliderFloat("intensity", &m_hbao->m_intensity, 0.0f, 10.0f);
		ImGui::SliderFloat("meters to viewspace unit transformation", &m_hbao->m_meters2viewspace, 0.0f, 10.0f);
		ImGui::SliderFloat("radius", &m_hbao->m_radius, 0.0f, 10.0f);

		//ImGui::Dummy(ImVec2(100, 200));
		ImGui::Dummy(ImVec2(0, 20));
		nex::gui::Separator(2.0f);
		//ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
		//	ImVec2(100.f, 120.f), ImColor(255, 255, 0, 255), "Hello World##HelloWorld!", 0, 0.0f, 0);

		ImGui::PopID();
	}
}