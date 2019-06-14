#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Shader.hpp>
#include <glm/glm.hpp>
#include <nex/post_processing/HBAO.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <nex/util/ExceptionHandling.hpp>
#include <nex/gui/ImGUI.hpp>
#include <random>
#include <nex/gui/Util.hpp>
#include <nex/shader/ShaderBuffer.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/Sampler.hpp>

// GCC under MINGW has no support for a real random device!
#if defined(__MINGW32__)  && defined(__GNUC__)
#include <boost/random/random_device.hpp>
#endif

#include <algorithm>
#include "nex/texture/Attachment.hpp"
#include "nex/texture/TextureManager.hpp"
#include <nex/material/Material.hpp>
#include "nex/drawing/StaticMeshDrawer.hpp"
#include "nex/math/Constant.hpp"

using namespace glm;

namespace nex
{

	HBAO::HBAO(unsigned int windowWidth,
		unsigned int windowHeight)
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
		m_hbao_ubo(0, sizeof(HBAOData), ShaderBuffer::UsageHint::DYNAMIC_COPY)

	{

		signed short hbaoRandomShort[HBAO_RANDOM_ELEMENTS * 4];

		for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i++)
		{
			float Rand1 = randomFloat(0, 1);
			float Rand2 = randomFloat(0, 1);

			// Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
			static float pi = static_cast<float>(nex::PI);
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

		TextureData data;
		data.colorspace = ColorSpace::RGBA;
		data.pixelDataType = PixelDataType::SHORT;
		data.internalFormat = InternFormat::RGBA16_SNORM;//RGBA16F RGBA16_SNORM
		data.minFilter = data.magFilter = TextureFilter::NearestNeighbor;

		m_hbao_random = std::make_unique<Texture2DArray>( HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, 1, data, hbaoRandomShort);
		m_hbao_randomview = Texture::createView(m_hbao_random.get(), TextureTarget::TEXTURE2D, 0, 1, 0, 1, data);


		m_aoDisplay = std::make_unique<DisplayTexPass>();
		m_bilateralBlur = std::make_unique<BilateralBlurPass>();
		m_depthLinearizer = std::make_unique<DepthLinearizerPass>();
		m_hbaoShader = std::make_unique<HbaoPass>();


		initRenderTargets(windowWidth, windowHeight);

		// initialize static shader attributes
		m_hbaoShader->setRamdomView(m_hbao_randomview.get());
		m_hbaoShader->setHbaoUBO(&m_hbao_ubo);
		
	}


	Texture2D * HBAO::getAO_Result()
	{
		return m_aoResultRT->getColor0AttachmentTexture();
	}

	Texture2D * HBAO::getBlurredResult()
	{
		return m_aoBlurredResultRT->getColor0AttachmentTexture();
	}

	void HBAO::onSizeChange(unsigned int newWidth, unsigned int newHeight)
	{
		this->windowWidth = newWidth;
		this->windowHeight = newHeight;

		initRenderTargets(windowWidth, windowHeight);
	}

	void HBAO::renderAO(Texture * depthTexture, const Projection& projection, bool blur)
	{
		unsigned int width = m_aoResultRT->getWidth();
		unsigned int height = m_aoResultRT->getHeight();
		auto* renderBackend = RenderBackend::get();

		prepareHbaoData(projection, width, height);


		renderBackend->setViewPort(0, 0, width, height);

		drawLinearDepth(depthTexture, projection);

		// draw hbao to hbao render target
		m_aoResultRT->bind();
		m_aoResultRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil

		m_hbaoShader->setHbaoData(std::move(m_hbaoDataSource));
		m_hbaoShader->setLinearDepth(m_depthLinearRT->getColorAttachments()[0].texture.get());
		m_hbaoShader->setRamdomView(m_hbao_randomview.get());
		m_hbaoShader->draw();


		if (blur) {
			// clear color/depth/stencil for all involved render targets
			m_aoBlurredResultRT->bind();
			m_aoBlurredResultRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil
			m_tempRT->bind();
			m_tempRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil

			// setup bilaterial blur and draw
			m_bilateralBlur->setLinearDepth(m_depthLinearRT->getColorAttachments()[0].texture.get());
			m_bilateralBlur->setSourceTexture(m_aoResultRT->getColorAttachments()[0].texture.get(), width, height);
			m_bilateralBlur->setSharpness(m_blur_sharpness);
			m_bilateralBlur->draw(m_tempRT.get(), m_aoBlurredResultRT.get());
		}
	}

	void HBAO::displayAOTexture(Texture* texture)
	{
		m_aoDisplay->setInputTexture(texture);
		m_aoDisplay->draw();
	}


	void HBAO::prepareHbaoData(const Projection & projection, int width, int height)
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

		const int useOrtho = projection.perspective ? 0 : 1;
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
		m_hbaoDataSource.PowExponent = std::max<float>(m_intensity, 0.0f);
		m_hbaoDataSource.NDotVBias = std::min<float>(std::max<float>(0.0f, m_bias), 1.0f);
		m_hbaoDataSource.AOMultiplier = 1.0f / (1.0f - m_hbaoDataSource.NDotVBias);

		// resolution
		const int quarterWidth = ((width + 3) / 4);
		const int quarterHeight = ((height + 3) / 4);

		m_hbaoDataSource.InvQuarterResolution = vec2(1.0f / float(quarterWidth), 1.0f / float(quarterHeight));
		m_hbaoDataSource.InvFullResolution = vec2(1.0f / float(width), 1.0f / float(height));
	}

	void HBAO::drawLinearDepth(Texture* depthTexture, const Projection & projection)
	{
		m_depthLinearRT->bind();
		m_depthLinearizer->setProjection(&projection);
		m_depthLinearizer->setInputTexture(depthTexture);
		m_depthLinearizer->draw();
	}

	void HBAO::initRenderTargets(unsigned int width, unsigned int height)
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

		m_depthLinearRT = std::make_unique<RenderTarget2D>(width, height, data, 1);

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
		m_aoResultRT = std::make_unique<RenderTarget2D>(width, height, data, 1);

		// m_aoBlurredResultRT
		m_aoBlurredResultRT = std::make_unique<RenderTarget2D>(width, height, data, 1);

		// m_tempRT
		m_tempRT = std::make_unique<RenderTarget2D>(width, height, data, 1);
	}

	BilateralBlurPass::BilateralBlurPass() :
		m_linearDepth(nullptr),
		m_sharpness(0),
		m_textureWidth(0),
		m_textureHeight(0),
		m_source(nullptr)
	{
		mShader = Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/bilateralblur.frag.glsl");

		auto state = mSampler.getState();
		state.minFilter = state.magFilter = TextureFilter::NearestNeighbor;
		mSampler.setState(state);
	}

	void BilateralBlurPass::setLinearDepth(Texture * linearDepth)
	{
		m_linearDepth = linearDepth;
	}

	void BilateralBlurPass::setSharpness(float sharpness)
	{
		m_sharpness = sharpness;
	}

	void BilateralBlurPass::setSourceTexture(Texture * source, unsigned int textureWidth, unsigned int textureHeight)
	{
		m_source = source;
		m_textureHeight = textureHeight;
		m_textureWidth = textureWidth;

		UniformLocation sourceLoc = mShader->getUniformLocation("texSource");
		mShader->setBinding(sourceLoc, 0);

		UniformLocation linearDepthLoc = mShader->getUniformLocation("texLinearDepth");
		mShader->setBinding(linearDepthLoc, 1);

	}

	void BilateralBlurPass::draw(RenderTarget2D* temp, RenderTarget2D* result)
	{
		temp->bind();
		bind();

		mShader->setTexture(m_source, &mSampler, 0); // TODO: check binding point!
		mShader->setTexture(m_linearDepth, &mSampler, 1); // TODO: check binding point!

		static UniformLocation sharpnessLoc = mShader->getUniformLocation("g_Sharpness");
		mShader->setFloat(sharpnessLoc, m_sharpness);

		// blur horizontal
		static UniformLocation invResolutionDirectionLoc = mShader->getUniformLocation("g_InvResolutionDirection");
		mShader->setVec2(invResolutionDirectionLoc, glm::vec2(1.0f / (float)m_textureWidth, 0));

		RenderState state = RenderState::createNoDepthTest();
		StaticMeshDrawer::drawFullscreenTriangle(state, this);

		// blur vertically
		result->bind();
		auto* renderResult = temp->getColorAttachments()[0].texture.get();

		// Note: mSampler is already bound to binding point 0!
		mShader->setTexture(renderResult, nullptr, 0); // TODO: check binding point!
		mShader->setVec2(invResolutionDirectionLoc, glm::vec2(0, 1.0f / (float)m_textureHeight));

		StaticMeshDrawer::drawFullscreenTriangle(state, this);
	}

	DepthLinearizerPass::DepthLinearizerPass() :
		m_input(nullptr),
		m_projection(nullptr)
	{
		mShader = Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/depthlinearize.frag.glsl");
		mSampler = std::make_unique<Sampler>(SamplerDesc());
		mSampler->setMinFilter(TextureFilter::NearestNeighbor);
		mSampler->setMagFilter(TextureFilter::NearestNeighbor);

		mSampler->setWrapR(TextureUVTechnique::ClampToBorder);
		mSampler->setWrapS(TextureUVTechnique::ClampToBorder);
		mSampler->setWrapT(TextureUVTechnique::ClampToBorder);
		mSampler->setBorderColor(glm::vec4(1.0));


		UniformLocation inputLoc = mShader->getUniformLocation("inputTexture");
		mShader->setBinding(inputLoc, 0);
	}

	void DepthLinearizerPass::draw()
	{
		bind();

		glm::vec4 vecData(m_projection->nearplane * m_projection->farplane,
			m_projection->nearplane - m_projection->farplane,
			m_projection->farplane,
			m_projection->perspective ? 1.0f : 0.0f);
		mShader->setVec4(0, vecData);

		mShader->setTexture(m_input, mSampler.get(), 0);
		
		RenderState state = RenderState::createNoDepthTest();
		StaticMeshDrawer::drawFullscreenTriangle(state, this);
	}

	void DepthLinearizerPass::setInputTexture(Texture * input)
	{
		m_input = input;
	}

	void DepthLinearizerPass::setProjection(const Projection* projection)
	{
		m_projection = projection;
	}

	DisplayTexPass::DisplayTexPass() : Pass(),
		m_input(nullptr)
	{
		mShader = Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/displaytex.frag.glsl");

		UniformLocation inputLoc = mShader->getUniformLocation("inputTexture");
		mShader->setBinding(inputLoc, 0);
	}

	void DisplayTexPass::draw()
	{
		bind();
		mShader->setTexture(m_input, &mSampler, 0); // TODO: check binding point!
		
		RenderState state = RenderState::createNoDepthTest();
		StaticMeshDrawer::drawFullscreenTriangle(state, this);
	}

	void DisplayTexPass::setInputTexture(Texture * input)
	{
		m_input = input;
	}

	HbaoPass::HbaoPass() :
		m_hbao_randomview(nullptr),
		m_linearDepth(nullptr),
		m_hbao_ubo(nullptr)
	{
		memset(&m_hbao_data, 0, sizeof(HBAOData));

		mShader = Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/hbao.frag.glsl");

		UniformLocation linearDepthLoc = mShader->getUniformLocation("texLinearDepth");
		mShader->setBinding(linearDepthLoc, 0);

		UniformLocation randomLoc = mShader->getUniformLocation("texRandom");
		mShader->setBinding(randomLoc, 1);

		mSampler.setMinFilter(TextureFilter::NearestNeighbor);
		mSampler.setMagFilter(TextureFilter::NearestNeighbor);
		mSampler.setWrapR(TextureUVTechnique::ClampToEdge);
		mSampler.setWrapS(TextureUVTechnique::ClampToEdge);
		mSampler.setWrapT(TextureUVTechnique::ClampToEdge);


		auto state = mSampler.getState();
		state.wrapR = state.wrapS = state.wrapT = TextureUVTechnique::Repeat;
		mPointSampler2.setState(state);
	}

	void HbaoPass::draw()
	{
		bind();
		m_hbao_ubo->bind();
		m_hbao_ubo->update(&m_hbao_data, sizeof(HBAOData));

		mShader->setTexture(m_linearDepth, &mSampler, 0);
		mShader->setTexture(m_hbao_randomview, &mPointSampler2, 1);

		RenderState state = RenderState::createNoDepthTest();
		StaticMeshDrawer::drawFullscreenTriangle(state, this);
	}

	void HbaoPass::setHbaoData(const HBAOData& hbao)
	{
		m_hbao_data = hbao;
	}

	void HbaoPass::setHbaoUBO(UniformBuffer* hbao_ubo)
	{
		m_hbao_ubo = hbao_ubo;
	}

	void HbaoPass::setLinearDepth(Texture * linearDepth)
	{
		m_linearDepth = linearDepth;
	}

	void HbaoPass::setRamdomView(Texture * randomView)
	{
		m_hbao_randomview = randomView;
	}


	float HBAO::getBlurSharpness() const
	{
		return m_blur_sharpness;
	}

	void HBAO::setBlurSharpness(float sharpness)
	{
		m_blur_sharpness = sharpness;
	}

	float HBAO::randomFloat(float a, float b)
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
		std::uniform_real_distribution<double> dist(a, b);
		std::random_device device;
		std::mt19937 gen(device());
		return dist(gen);

#endif
	}

	float nex::HBAO::lerp(float a, float b, float f) {
		return a + f * (b - a);
	}

	HbaoConfigurationView::HbaoConfigurationView(HBAO * hbao) : m_hbao(hbao), m_parent(nullptr), m_test(0)
	{
		m_isVisible = true;
	}

	void HbaoConfigurationView::drawSelf()
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