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
#include <nex/buffer/ShaderBuffer.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/Sampler.hpp>

// GCC under MINGW has no support for a real random device!
#if defined(__MINGW32__)  && defined(__GNUC__)
#include <boost/random/random_device.hpp>
#endif

#include <algorithm>
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
		m_hbao_ubo(0, sizeof(HBAOData), nullptr, ShaderBuffer::UsageHint::DYNAMIC_COPY)

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
			mHbaoRandom[i].x = cosf(Angle);
			mHbaoRandom[i].y = sinf(Angle);
			mHbaoRandom[i].z = Rand2;
			mHbaoRandom[i].w = 0;
#define SCALE ((1<<15))
			hbaoRandomShort[i * 4 + 0] = (signed short)(SCALE* mHbaoRandom[i].x);
			hbaoRandomShort[i * 4 + 1] = (signed short)(SCALE* mHbaoRandom[i].y);
			hbaoRandomShort[i * 4 + 2] = (signed short)(SCALE* mHbaoRandom[i].z);
			hbaoRandomShort[i * 4 + 3] = (signed short)(SCALE* mHbaoRandom[i].w);
#undef SCALE
		}



		TextureDesc data;
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

		mHbaoDeinterleavedPass = std::make_unique<HbaoDeinterleavedPass>();
		mViewNormalPass = std::make_unique<ViewNormalPass>();
		mDeinterleavePass = std::make_unique<DeinterleavePass>();
		mReinterleavePass = std::make_unique<ReinterleavePass>();
		mHbaoBlur = std::make_unique<HbaoBlur>();

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

	nex::Texture2D* HBAO::getViewSpaceNormals()
	{
		return (Texture2D*)mViewSpaceNormalsRT->getColorAttachmentTexture(0);
	}

	nex::Texture2D* HBAO::getLinearDepth()
	{
		return (Texture2D*)m_depthLinearRT->getColorAttachmentTexture(0);
	}

	Texture* HBAO::getDepthView(int index)
	{
		if (index >= HBAO_RANDOM_ELEMENTS) throw_with_trace(std::runtime_error("HBAO::getDepthView: index out of bound"));
		return mDepthView4th[index].get();
	}

	Texture* HBAO::getAoResultView4th(int index)
	{
		if (index >= HBAO_RANDOM_ELEMENTS) throw_with_trace(std::runtime_error("HBAO::getDepthView: index out of bound"));
		return mHbaoResultView4th[index].get();
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


		renderCacheAwareAO(depthTexture, projection, blur);

		return;

		prepareHbaoData(projection, width, height);


		renderBackend->setViewPort(0, 0, width, height);

		drawLinearDepth(depthTexture, projection);

		// draw hbao to hbao render target
		m_aoResultRT->bind();
		//m_aoResultRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil

		m_hbaoShader->bind();

		
		m_hbao_ubo.update(sizeof(HBAOData), &m_hbaoDataSource);
		m_hbaoShader->setHbaoUBO(&m_hbao_ubo);

		m_hbaoShader->setLinearDepth(m_depthLinearRT->getColorAttachments()[0].texture.get());
		m_hbaoShader->setRamdomView(m_hbao_randomview.get());

		RenderState state = RenderState::createNoDepthTest();
		StaticMeshDrawer::drawFullscreenTriangle(state, m_hbaoShader.get());


		if (blur) {
			// clear color/depth/stencil for all involved render targets
			//m_aoBlurredResultRT->bind();
			//m_aoBlurredResultRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil
			m_tempRT->bind();
			//m_tempRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil

			// setup bilaterial blur and draw
			/*m_bilateralBlur->setLinearDepth(m_depthLinearRT->getColorAttachments()[0].texture.get());
			m_bilateralBlur->setSourceTexture(m_aoResultRT->getColorAttachments()[0].texture.get(), width, height);
			m_bilateralBlur->setSharpness(m_blur_sharpness / m_meters2viewspace);
			m_bilateralBlur->draw(m_tempRT.get(), m_aoBlurredResultRT.get());*/


			RenderState state = RenderState::createNoDepthTest();
			mHbaoBlur->bindPreset(0);
			mHbaoBlur->setSource(m_aoResultRT->getColorAttachments()[0].texture.get());
			mHbaoBlur->setSharpness(m_blur_sharpness / m_meters2viewspace);
			mHbaoBlur->setInvResolutionDirection({1.0f / float(width), 1.0f / float(height)});
			StaticMeshDrawer::drawFullscreenTriangle(state, mHbaoBlur->getPreset(0));

			m_aoBlurredResultRT->bind();
			mHbaoBlur->bindPreset(1);
			mHbaoBlur->setSource(m_tempRT->getColorAttachments()[0].texture.get());
			mHbaoBlur->setSharpness(m_blur_sharpness / m_meters2viewspace);
			mHbaoBlur->setInvResolutionDirection({ 1.0f / float(width), 1.0f / float(height) });
			StaticMeshDrawer::drawFullscreenTriangle(state, mHbaoBlur->getPreset(1));
		}
	}

	void HBAO::renderCacheAwareAO(Texture* depth, const Projection& projection, bool blur)
	{
		const auto width = m_aoResultRT->getWidth();
		const auto height = m_aoResultRT->getHeight();
		const auto quarterWidth = (width + 3) / 4;
		const auto quarterHeight = (height + 3) / 4;
		auto* renderBackend = RenderBackend::get();

		prepareHbaoData(projection, width, height);

		renderBackend->setViewPort(0, 0, width, height);
		drawLinearDepth(depth, projection);

		auto* linearDepth = m_depthLinearRT->getColorAttachmentTexture(0);
		RenderState state;
		state.doDepthTest = false;

		//viewspace normals
		{
			mViewSpaceNormalsRT->bind();
			mViewNormalPass->bind();
			mViewNormalPass->setLinearDepth(linearDepth);
			mViewNormalPass->setInvFullResolution(m_hbaoDataSource.InvFullResolution);
			mViewNormalPass->setProjInfo(m_hbaoDataSource.projInfo);
			mViewNormalPass->setProjOrtho(m_hbaoDataSource.projOrtho);

			StaticMeshDrawer::drawFullscreenTriangle(state, mViewNormalPass.get());
		}

		//deinterleave
		{
			mDeinterleaveRT->bind();
			renderBackend->setViewPort(0, 0, quarterWidth, quarterHeight);
			mDeinterleavePass->bind();
			mDeinterleavePass->setLinearDepth(linearDepth);

			for (auto i = 0; i < HBAO_RANDOM_ELEMENTS; i+= NUM_MRT) {
				mDeinterleavePass->setInfo({ 
					float(i % 4) + 0.5f, 
					float(i / 4) + 0.5f, // / 
					m_hbaoDataSource.InvFullResolution.x,
					m_hbaoDataSource.InvFullResolution.y}
				);

				int index = i / NUM_MRT;
				mDeinterleaveRT->resetAttachments(mDeinterleaveAttachment[index]);
				StaticMeshDrawer::drawFullscreenTriangle(state, mDeinterleavePass.get());
			}
		}

		//calc hbao
		{
			mCacheAwareAoRT->bind();
			renderBackend->setViewPort(0, 0, quarterWidth, quarterHeight);
			mHbaoDeinterleavedPass->bind();

			m_hbao_ubo.update(sizeof(HBAOData), &m_hbaoDataSource);
			mHbaoDeinterleavedPass->setHbaoUBO(&m_hbao_ubo);
			mHbaoDeinterleavedPass->setLinearDepth(mDepthArray4th.get());
			mHbaoDeinterleavedPass->setViewNormals(mViewSpaceNormalsRT->getColorAttachmentTexture(0));
			mHbaoDeinterleavedPass->setImageOutput(mHbaoResultArray4th.get());

			renderBackend->drawArray(state, Topology::TRIANGLES, 0, 3 * HBAO_RANDOM_ELEMENTS);
			renderBackend->syncMemoryWithGPU(MemorySync_TextureUpdate | MemorySync_ShaderImageAccess);
		}

		//reinterleave
		{
			
			m_aoResultRT->bind();
			renderBackend->setViewPort(0, 0, width, height);
			m_aoResultRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil

			mReinterleavePass->bind();
			mReinterleavePass->setTextureResultArray(mHbaoResultArray4th.get());
			StaticMeshDrawer::drawFullscreenTriangle(state, mReinterleavePass.get());
		}

		//blur
		if (blur) {

			// clear color/depth/stencil for all involved render targets
			//m_aoBlurredResultRT->bind();
			//m_aoBlurredResultRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil
			m_tempRT->bind();
			//m_tempRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil

			// setup bilaterial blur and draw
			/*m_bilateralBlur->setLinearDepth(m_depthLinearRT->getColorAttachments()[0].texture.get());
			m_bilateralBlur->setSourceTexture(m_aoResultRT->getColorAttachments()[0].texture.get(), width, height);
			m_bilateralBlur->setSharpness(m_blur_sharpness / m_meters2viewspace);
			m_bilateralBlur->draw(m_tempRT.get(), m_aoBlurredResultRT.get());*/


			RenderState state = RenderState::createNoDepthTest();
			mHbaoBlur->bindPreset(0);
			mHbaoBlur->setSource(m_aoResultRT->getColorAttachments()[0].texture.get());
			mHbaoBlur->setSharpness(m_blur_sharpness / m_meters2viewspace);
			mHbaoBlur->setInvResolutionDirection({ 1.0f / float(width), 1.0f / float(height) });
			StaticMeshDrawer::drawFullscreenTriangle(state, mHbaoBlur->getPreset(0));

			m_aoBlurredResultRT->bind();
			mHbaoBlur->bindPreset(1);
			mHbaoBlur->setSource(m_tempRT->getColorAttachments()[0].texture.get());
			mHbaoBlur->setSharpness(m_blur_sharpness / m_meters2viewspace);
			mHbaoBlur->setInvResolutionDirection({ 1.0f / float(width), 1.0f / float(height) });
			StaticMeshDrawer::drawFullscreenTriangle(state, mHbaoBlur->getPreset(1));
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
			-1.0f / P[4 * 0 + 0],
			-1.0f / P[4 * 1 + 1]
			
			//-(1.0f - P[4 * 2 + 0]) / P[4 * 0 + 0], // L/N
			//-(1.0f + P[4 * 2 + 1]) / P[4 * 1 + 1], // B/N
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


		for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i++) {
			m_hbaoDataSource.float2Offsets[i] = vec4(float(i % 4) + 0.5f, float(i / 4) + 0.5f, 0, 0);
			m_hbaoDataSource.jitters[i] = mHbaoRandom[i];
		}
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
		TextureDesc depthDesc;
		depthDesc.internalFormat = InternFormat::R32F;
		depthDesc.magFilter = TextureFilter::NearestNeighbor;
		depthDesc.minFilter = TextureFilter::NearestNeighbor;
		depthDesc.wrapR = TextureUVTechnique::ClampToEdge;
		depthDesc.wrapS = TextureUVTechnique::ClampToEdge;
		depthDesc.wrapT = TextureUVTechnique::ClampToEdge;
		depthDesc.pixelDataType = PixelDataType::FLOAT;
		depthDesc.generateMipMaps = false;
		depthDesc.colorspace = ColorSpace::R;

		m_depthLinearRT = std::make_unique<RenderTarget2D>(width, height, depthDesc, 1);

		// m_aoResultRT
		TextureDesc aoData; 
		aoData.internalFormat = InternFormat::RG16F;
		aoData.useSwizzle = true;
		//aoData.swizzle = { Channel::RED, Channel::RED, Channel::RED, Channel::RED };
		aoData.swizzle = { Channel::RED, Channel::GREEN, Channel::ZERO, Channel::ZERO };
		aoData.minFilter = TextureFilter::NearestNeighbor;
		aoData.magFilter = TextureFilter::NearestNeighbor;
		aoData.wrapR = TextureUVTechnique::ClampToEdge;
		aoData.wrapS = TextureUVTechnique::ClampToEdge;
		aoData.wrapT = TextureUVTechnique::ClampToEdge;
		aoData.pixelDataType = PixelDataType::FLOAT_HALF;
		aoData.generateMipMaps = false;
		aoData.colorspace = ColorSpace::RG;
		m_aoResultRT = std::make_unique<RenderTarget2D>(width, height, aoData, 1);

		// m_aoBlurredResultRT
		m_aoBlurredResultRT = std::make_unique<RenderTarget2D>(width, height, aoData, 1);

		// m_tempRT
		m_tempRT = std::make_unique<RenderTarget2D>(width, height, aoData, 1);


		//cache aware stuff
		int quarterWidth = ((width + 3) / 4);
		int quarterHeight = ((height + 3) / 4);

		mDepthArray4th = std::make_unique<Texture2DArray>(quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS, depthDesc, nullptr);
		mHbaoResultArray4th = std::make_unique<Texture2DArray>(quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS, aoData, nullptr);
		mDeinterleaveRT = std::make_unique<RenderTarget>(quarterWidth, quarterHeight);

		for (auto i = 0; i < HBAO_RANDOM_ELEMENTS / NUM_MRT; ++i) {
			mDeinterleaveAttachment[i].resize(NUM_MRT);
		}

		TextureDesc test;// = aoData;
		test.useSwizzle = false;

		for (auto i = 0; i < HBAO_RANDOM_ELEMENTS; ++i) {
			mDepthView4th[i] = Texture::createView(mDepthArray4th.get(), TextureTarget::TEXTURE2D, 0, 1, i, 1, depthDesc);
			mHbaoResultView4th[i] = Texture::createView(mHbaoResultArray4th.get(), TextureTarget::TEXTURE2D, 0, 1, i, 1, aoData);
		}

		for (auto i = 0; i < HBAO_RANDOM_ELEMENTS / NUM_MRT; ++i) {
			mDeinterleaveAttachment[i].resize(NUM_MRT);
			for (auto j = 0; j < NUM_MRT; ++j) {
				mDeinterleaveAttachment[i][j].colorAttachIndex = j;
				mDeinterleaveAttachment[i][j].texture = mDepthView4th[i * NUM_MRT + j];
			}
		}

		mDeinterleaveRT->resetAttachments(mDeinterleaveAttachment[1]);

		mCacheAwareAoRT = std::make_unique<RenderTarget>(quarterWidth, quarterHeight);


		TextureDesc normalsDesc;
		normalsDesc.internalFormat = InternFormat::RGBA8;
		normalsDesc.magFilter = TextureFilter::NearestNeighbor;
		normalsDesc.minFilter = TextureFilter::NearestNeighbor;
		normalsDesc.wrapR = TextureUVTechnique::ClampToEdge;
		normalsDesc.wrapS = TextureUVTechnique::ClampToEdge;
		normalsDesc.wrapT = TextureUVTechnique::ClampToEdge;
		normalsDesc.pixelDataType = PixelDataType::UBYTE;
		normalsDesc.generateMipMaps = false;
		normalsDesc.colorspace = ColorSpace::RGBA;
		mViewSpaceNormalsRT = std::make_unique<RenderTarget2D>(width, height, normalsDesc, 1);
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

		mShader->setTexture(m_source, &mSampler, 0);
		mShader->setTexture(m_linearDepth, &mSampler, 1); 

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

	HbaoPass::HbaoPass()
	{
		std::vector<std::string> defines = { "#define AO_DEINTERLEAVED 0", "#define AO_BLUR 1" };

		mShader = Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/hbao_fs.glsl",
			nullptr, nullptr, nullptr, defines);

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

	void HbaoPass::setHbaoUBO(UniformBuffer* hbao_ubo)
	{
		hbao_ubo->bindToTarget(0);
	}

	void HbaoPass::setLinearDepth(Texture * linearDepth)
	{
		mShader->setTexture(linearDepth, &mSampler, 0);
	}

	void HbaoPass::setRamdomView(Texture * randomView)
	{
		mShader->setTexture(randomView, &mPointSampler2, 1);
	}


	HbaoDeinterleavedPass::HbaoDeinterleavedPass()
	{
		std::vector<std::string> defines = { "#define AO_DEINTERLEAVED 1", "#define AO_BLUR 1" };

		mShader = Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/hbao_fs.glsl",
			nullptr, nullptr, nullptr, defines);

		//"post_processing/hbao/fullscreenquad.geo.glsl"

		mLinearDepth = mShader->createTextureUniform("texLinearDepth", UniformType::TEXTURE2D_ARRAY, 0);
		mViewNormals = mShader->createTextureUniform("texViewNormal", UniformType::TEXTURE2D, 1);
		mImgOutput = { mShader->getUniformLocation("imgOutput"), UniformType::IMAGE2D_ARRAY, 0 };



		mSampler.setMinFilter(TextureFilter::NearestNeighbor);
		mSampler.setMagFilter(TextureFilter::NearestNeighbor);
		mSampler.setWrapR(TextureUVTechnique::ClampToEdge);
		mSampler.setWrapS(TextureUVTechnique::ClampToEdge);
		mSampler.setWrapT(TextureUVTechnique::ClampToEdge);
	}
	void HbaoDeinterleavedPass::setHbaoUBO(UniformBuffer* hbao_ubo)
	{
		hbao_ubo->bindToTarget(0);
	}
	void HbaoDeinterleavedPass::setLinearDepth(Texture* linearDepth)
	{
		mShader->setTexture(linearDepth, &mSampler, mLinearDepth.bindingSlot);
	}
	void HbaoDeinterleavedPass::setViewNormals(Texture* normals)
	{
		mShader->setTexture(normals, &mSampler, mViewNormals.bindingSlot);
	}

	void HbaoDeinterleavedPass::setImageOutput(Texture* imgOutput)
	{
		mShader->setImageLayerOfTexture(mImgOutput.location, 
			imgOutput, 
			mImgOutput.location, 
			TextureAccess::WRITE_ONLY, 
			InternFormat::RG16F, 
			0, 
			true, 0);
	}




	ViewNormalPass::ViewNormalPass()
	{
		mShader = Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/viewnormal_fs.glsl");
		mProjInfo = {mShader->getUniformLocation("projInfo"), UniformType::VEC4};
		mProjOrtho = { mShader->getUniformLocation("projOrtho"), UniformType::INT };
		mInvFullResolution = { mShader->getUniformLocation("InvFullResolution"), UniformType::VEC2 };
		mLinearDepth = mShader->createTextureUniform("texLinearDepth", UniformType::TEXTURE2D, 0);
	}

	ViewNormalPass::~ViewNormalPass() = default;

	void ViewNormalPass::setProjInfo(const glm::vec4 & projInfo)
	{
		mShader->setVec4(mProjInfo.location, projInfo);
	}
	void ViewNormalPass::setProjOrtho(bool projOrtho)
	{
		mShader->setInt(mProjOrtho.location, static_cast<int>(projOrtho));
	}
	void ViewNormalPass::setInvFullResolution(const glm::vec2 & invFullResolution)
	{
		mShader->setVec2(mInvFullResolution.location, invFullResolution);
	}
	void ViewNormalPass::setLinearDepth(Texture * linearDepth)
	{
		mShader->setTexture(linearDepth, &mSampler, mLinearDepth.bindingSlot);
	}


	DeinterleavePass::DeinterleavePass()
	{
		mShader = Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/hbao_deinterleave_fs.glsl");
		mInfo = { mShader->getUniformLocation("info"), UniformType::VEC4 };
		mLinearDepth = mShader->createTextureUniform("texLinearDepth", UniformType::TEXTURE2D, 0);
	}
	DeinterleavePass::~DeinterleavePass() = default;

	void DeinterleavePass::setInfo(const glm::vec4 & info)
	{
		mShader->setVec4(mInfo.location, info);
	}

	void DeinterleavePass::setLinearDepth(Texture* linearDepth)
	{
		mShader->setTexture(linearDepth, &mSampler, mLinearDepth.bindingSlot);
	}

	ReinterleavePass::ReinterleavePass()
	{
		mShader = Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", "post_processing/hbao/hbao_reinterleave_fs.glsl",
			nullptr, nullptr, nullptr, {"#define AO_BLUR 1"});
		mResultArray = mShader->createTextureUniform("texResultsArray", UniformType::TEXTURE2D_ARRAY, 0);

		mSampler.setMinFilter(TextureFilter::NearestNeighbor);
		mSampler.setMagFilter(TextureFilter::NearestNeighbor);
	}

	ReinterleavePass::~ReinterleavePass() = default;

	void ReinterleavePass::setTextureResultArray(Texture * resultArray)
	{
		mShader->setTexture(resultArray, &mSampler, mResultArray.bindingSlot);
	}

	HbaoBlur::HbaoBlur() : mActivePreset(0)
	{
		mBlurPreset[0] = std::make_unique<Pass>(Shader::create("post_processing/hbao/fullscreenquad.vert.glsl", 
			"post_processing/hbao/hbao_blur_fs.glsl",
			nullptr, nullptr, nullptr, {"#define AO_BLUR_PRESENT 0"}));

		mBlurPreset[1] = std::make_unique<Pass>(Shader::create("post_processing/hbao/fullscreenquad.vert.glsl",
			"post_processing/hbao/hbao_blur_fs.glsl",
			nullptr, nullptr, nullptr, { "#define AO_BLUR_PRESENT 1" }));

		mSharpness[0] = { mBlurPreset[0]->getShader()->getUniformLocation("g_Sharpness"), UniformType::FLOAT};
		mSharpness[1] = { mBlurPreset[1]->getShader()->getUniformLocation("g_Sharpness"), UniformType::FLOAT };

		mInvResolutionDirection[0] = { mBlurPreset[0]->getShader()->getUniformLocation("g_InvResolutionDirection"), UniformType::VEC2 };
		mInvResolutionDirection[1] = { mBlurPreset[1]->getShader()->getUniformLocation("g_InvResolutionDirection"), UniformType::VEC2 };

		mSource.bindingSlot = 0;
	}
	HbaoBlur::~HbaoBlur() = default;

	void HbaoBlur::bindPreset(int id)
	{
		if (id != 0 && id != 1) throw_with_trace(std::runtime_error("HbaoBlur::bindPreset: id has to be 0 or 1"));
		mActivePreset = id;
	}

	nex::Pass* HbaoBlur::getPreset(int id)
	{
		return mBlurPreset[id].get();
	}

	void HbaoBlur::setSharpness(float sharpness)
	{
		mBlurPreset[mActivePreset]->getShader()->setFloat(mSharpness[mActivePreset].location, sharpness);
	}

	void HbaoBlur::setInvResolutionDirection(const glm::vec2 & invResolustion)
	{
		mBlurPreset[mActivePreset]->getShader()->setVec2(mInvResolutionDirection[mActivePreset].location, invResolustion);
	}

	void HbaoBlur::setSource(Texture* source)
	{
		auto* sampler = TextureManager::get()->getPointSampler();
		mBlurPreset[mActivePreset]->getShader()->setTexture(source, sampler, mSource.bindingSlot);
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
		mIsVisible = true;
	}

	void HbaoConfigurationView::drawSelf()
	{
		// render configuration properties
		ImGui::PushID(mId.c_str());
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