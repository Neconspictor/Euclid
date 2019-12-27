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
#include "nex/renderer/Drawer.hpp"
#include "nex/math/Constant.hpp"
#include <imgui/imgui_internal.h>

using namespace glm;

namespace nex
{

	HBAO::HBAO(unsigned int windowWidth,
		unsigned int windowHeight,
		bool useSpecialBlur)
		:
		mBlurSharpness(40.0f),
		mMeters2ViewSpace(1.0f),
		mRadius(2.0f),
		mIntensity(1.5f),
		mBias(0.1f),
		mWindowWidth(windowWidth),
		mWindowHeight(windowHeight),
		mViewSpaceZRT(nullptr),
		mAoResultRT(nullptr),
		mTempRT(nullptr),
		mHbaoUbo(0, sizeof(HBAOData), nullptr, ShaderBuffer::UsageHint::DYNAMIC_COPY),
		mUseSpecialBlur(useSpecialBlur),
		mBlurAo(true),
		mUseDeinterleavedTexturing(true),
		mBlurKernelRadius(0)

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
		data.internalFormat = InternalFormat::RGBA16_SNORM;//RGBA16F RGBA16_SNORM
		data.minFilter = data.magFilter = TexFilter::Nearest;

		mHbaoRandomTexture = std::make_unique<Texture2DArray>( HBAO_RANDOMTEX_SIZE, HBAO_RANDOMTEX_SIZE, 1, data, hbaoRandomShort);
		mHbaoRandomview = Texture::createView(mHbaoRandomTexture.get(), TextureTarget::TEXTURE2D, 0, 1, 0, 1, data);


		mAoDisplayShader = std::make_unique<DisplayAoShader>();
		mDepthToViewSpaceZ = std::make_unique<DepthToViewSpaceZShader>();
		mHbaoShader = std::make_unique<HbaoShader>();

		mHbaoDeinterleavedShader = std::make_unique<HbaoDeinterleavedShader>();
		mViewNormalShader = std::make_unique<ViewNormalShader>();
		mDeinterleaveShader = std::make_unique<DeinterleaveShader>();
		mReinterleaveShader = std::make_unique<ReinterleaveShader>();


		setBlurKernelRadius(3);

		initRenderTargets(windowWidth, windowHeight);

		// initialize static shader attributes
		mHbaoShader->setRamdomView(mHbaoRandomview.get());
		mHbaoShader->setHbaoUBO(&mHbaoUbo);
	}


	Texture2D * HBAO::getAO_Result()
	{
		return mAoResultRT->getColor0AttachmentTexture();
	}

	Texture2D * HBAO::getBlurredResult()
	{
		return mAoBlurredResultRT->getColor0AttachmentTexture();
	}

	nex::Texture2D* HBAO::getViewSpaceNormals()
	{
		return (Texture2D*)mViewSpaceNormalsRT->getColorAttachmentTexture(0);
	}

	nex::Texture2D* HBAO::getViewSpaceZ()
	{
		return (Texture2D*)mViewSpaceZRT->getColorAttachmentTexture(0);
	}

	const Projection& HBAO::getViewSpaceZProjectionInfo() const
	{
		return mDepthToViewSpaceZ->getLastProjectionData();
	}

	Texture* HBAO::getViewSpaceZ4thView(int index)
	{
		if (index >= HBAO_RANDOM_ELEMENTS) throw_with_trace(std::runtime_error("HBAO::getViewSpaceZ4thView: index out of bound"));
		return mDepthView4th[index].get();
	}

	Texture* HBAO::getAoResultView4th(int index)
	{
		if (index >= HBAO_RANDOM_ELEMENTS) throw_with_trace(std::runtime_error("HBAO::getViewSpaceZ4thView: index out of bound"));
		return mHbaoResultView4th[index].get();
	}

	void HBAO::onSizeChange(unsigned int newWidth, unsigned int newHeight)
	{
		mWindowWidth = newWidth;
		mWindowHeight = newHeight;

		initRenderTargets(mWindowWidth, mWindowHeight);
	}

	void HBAO::renderAO(const Texture * depthTexture, const Projection& projection)
	{
		
		if (mUseDeinterleavedTexturing) {
			renderWithDeinterleavedAO(depthTexture, projection);
		}
		else {
			renderNonDeinterleavedAO(depthTexture, projection);
		}

		//blur
		if (mBlurAo) {

			// clear color/depth/stencil for all involved render targets
			//mAoBlurredResultRT->bind();
			//mAoBlurredResultRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil
			
			//mTempRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil

			// setup bilaterial blur and draw

			unsigned int width = mAoResultRT->getWidth();
			unsigned int height = mAoResultRT->getHeight();

			if (mUseSpecialBlur) {

				mHbaoBlur->draw(mTempRT.get(), 
					mAoBlurredResultRT.get(), 
					mAoResultRT->getColorAttachments()[0].texture.get(), 
					width, 
					height, 
					mBlurSharpness / mMeters2ViewSpace);
			}
			else {
				mBilateralBlur->draw(mTempRT.get(), mAoBlurredResultRT.get(), mViewSpaceZRT->getColorAttachments()[0].texture.get(),
					mAoResultRT->getColorAttachments()[0].texture.get(), width, height, mBlurSharpness / mMeters2ViewSpace);
			}
		}
	}


	void HBAO::displayAOTexture(const Texture* ao)
	{
		mAoDisplayShader->draw(ao);
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

		float R = mRadius * mMeters2ViewSpace;
		m_hbaoDataSource.R2 = R * R;
		m_hbaoDataSource.NegInvR2 = -1.0f / m_hbaoDataSource.R2;
		m_hbaoDataSource.RadiusToScreen = R * 0.5f * projScale;

		// ao
		m_hbaoDataSource.PowExponent = std::max<float>(mIntensity, 0.0f);
		m_hbaoDataSource.NDotVBias = std::min<float>(std::max<float>(0.0f, mBias), 1.0f);
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

	void HBAO::reloadBlurShaders(unsigned radius)
	{
		mBilateralBlur = std::make_unique<BilateralBlurShader>(radius);
		mHbaoBlur = std::make_unique<HbaoBlur>(radius);
	}


	void HBAO::renderWithDeinterleavedAO(const Texture* depth, const Projection& projection)
	{
		const auto width = mAoResultRT->getWidth();
		const auto height = mAoResultRT->getHeight();
		const auto quarterWidth = (width + 3) / 4;
		const auto quarterHeight = (height + 3) / 4;
		auto* renderBackend = RenderBackend::get();

		prepareHbaoData(projection, width, height);

		renderBackend->setViewPort(0, 0, width, height);
		drawLinearDepth(depth, projection);

		auto* linearDepth = mViewSpaceZRT->getColorAttachmentTexture(0);
		RenderState state;
		state.doDepthTest = false;

		//viewspace normals
		{
			mViewSpaceNormalsRT->bind();
			mViewNormalShader->bind();
			mViewNormalShader->setLinearDepth(linearDepth);
			mViewNormalShader->setInvFullResolution(m_hbaoDataSource.InvFullResolution);
			mViewNormalShader->setProjInfo(m_hbaoDataSource.projInfo);
			mViewNormalShader->setProjOrtho(m_hbaoDataSource.projOrtho);

			Drawer::drawFullscreenTriangle(state, mViewNormalShader.get());
		}

		//deinterleave
		{
			mDeinterleaveRT->bind();
			renderBackend->setViewPort(0, 0, quarterWidth, quarterHeight);
			mDeinterleaveShader->bind();
			mDeinterleaveShader->setLinearDepth(linearDepth);

			for (auto i = 0; i < HBAO_RANDOM_ELEMENTS; i += NUM_MRT) {
				mDeinterleaveShader->setInfo({
					float(i % 4) + 0.5f,
					float(i / 4) + 0.5f, // / 
					m_hbaoDataSource.InvFullResolution.x,
					m_hbaoDataSource.InvFullResolution.y }
				);

				int index = i / NUM_MRT;
				mDeinterleaveRT->resetAttachments(mDeinterleaveAttachment[index]);

				Drawer::drawFullscreenTriangle(state, mDeinterleaveShader.get());
			}
		}

		//calc hbao
		{
			mCacheAwareAoRT->bind();
			renderBackend->setViewPort(0, 0, quarterWidth, quarterHeight);
			mHbaoDeinterleavedShader->bind();

			mHbaoUbo.update(sizeof(HBAOData), &m_hbaoDataSource);
			mHbaoDeinterleavedShader->setHbaoUBO(&mHbaoUbo);
			mHbaoDeinterleavedShader->setLinearDepth(mDepthArray4th.get());
			mHbaoDeinterleavedShader->setViewNormals(mViewSpaceNormalsRT->getColorAttachmentTexture(0));
			mHbaoDeinterleavedShader->setImageOutput(mHbaoResultArray4th.get());

			renderBackend->drawArray(state, Topology::TRIANGLES, 0, 3 * HBAO_RANDOM_ELEMENTS);
			renderBackend->syncMemoryWithGPU(MemorySync_TextureUpdate | MemorySync_ShaderImageAccess);
		}

		//reinterleave
		{

			mAoResultRT->bind();
			renderBackend->setViewPort(0, 0, width, height);
			mAoResultRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil

			mReinterleaveShader->bind();
			mReinterleaveShader->setTextureResultArray(mHbaoResultArray4th.get());
			Drawer::drawFullscreenTriangle(state, mReinterleaveShader.get());
		}
	}

	void HBAO::renderNonDeinterleavedAO(const Texture* depth, const Projection& projection)
	{
		unsigned int width = mAoResultRT->getWidth();
		unsigned int height = mAoResultRT->getHeight();
		auto* renderBackend = RenderBackend::get();

		prepareHbaoData(projection, width, height);


		renderBackend->setViewPort(0, 0, width, height);

		drawLinearDepth(depth, projection);

		// draw hbao to hbao render target
		mAoResultRT->bind();
		//mAoResultRT->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil

		mHbaoShader->bind();


		mHbaoUbo.update(sizeof(HBAOData), &m_hbaoDataSource);
		mHbaoShader->setHbaoUBO(&mHbaoUbo);

		mHbaoShader->setLinearDepth(mViewSpaceZRT->getColorAttachments()[0].texture.get());
		mHbaoShader->setRamdomView(mHbaoRandomview.get());

		const auto& state = RenderState::getNoDepthTest();
		Drawer::drawFullscreenTriangle(state, mHbaoShader.get());
	}

	void HBAO::drawLinearDepth(const Texture* depthTexture, const Projection & projection)
	{
		mViewSpaceZRT->bind();
		mDepthToViewSpaceZ->draw(projection, depthTexture);
	}

	void HBAO::initRenderTargets(unsigned int width, unsigned int height)
	{
		//at first release gpu memory before acquiring new memory
		mViewSpaceZRT = nullptr;
		mAoResultRT = nullptr;
		mAoBlurredResultRT = nullptr;
		mTempRT = nullptr;

		// mViewSpaceZRT
		TextureDesc depthDesc;
		depthDesc.internalFormat = InternalFormat::R32F;
		depthDesc.magFilter = TexFilter::Nearest;
		depthDesc.minFilter = TexFilter::Nearest;
		depthDesc.wrapR = UVTechnique::ClampToEdge;
		depthDesc.wrapS = UVTechnique::ClampToEdge;
		depthDesc.wrapT = UVTechnique::ClampToEdge;
		depthDesc.pixelDataType = PixelDataType::FLOAT;
		depthDesc.generateMipMaps = false;
		depthDesc.colorspace = ColorSpace::R;

		mViewSpaceZRT = std::make_unique<RenderTarget2D>(width, height, depthDesc, 1);

		// mAoResultRT
		TextureDesc aoData; 
		aoData.internalFormat = InternalFormat::RG16F;
		aoData.useSwizzle = true;
		//aoData.swizzle = { Channel::RED, Channel::RED, Channel::RED, Channel::RED };
		aoData.swizzle = { Channel::RED, Channel::GREEN, Channel::ZERO, Channel::ZERO };
		aoData.minFilter = TexFilter::Nearest;
		aoData.magFilter = TexFilter::Nearest;
		aoData.wrapR = UVTechnique::ClampToEdge;
		aoData.wrapS = UVTechnique::ClampToEdge;
		aoData.wrapT = UVTechnique::ClampToEdge;
		aoData.pixelDataType = PixelDataType::FLOAT_HALF;
		aoData.generateMipMaps = false;
		aoData.colorspace = ColorSpace::RG;
		mAoResultRT = std::make_unique<RenderTarget2D>(width, height, aoData, 1);

		// mAoBlurredResultRT
		mAoBlurredResultRT = std::make_unique<RenderTarget2D>(width, height, aoData, 1);

		// mTempRT
		mTempRT = std::make_unique<RenderTarget2D>(width, height, aoData, 1);


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
		normalsDesc.internalFormat = InternalFormat::RGBA8;
		normalsDesc.magFilter = TexFilter::Nearest;
		normalsDesc.minFilter = TexFilter::Nearest;
		normalsDesc.wrapR = UVTechnique::ClampToEdge;
		normalsDesc.wrapS = UVTechnique::ClampToEdge;
		normalsDesc.wrapT = UVTechnique::ClampToEdge;
		normalsDesc.pixelDataType = PixelDataType::UBYTE;
		normalsDesc.generateMipMaps = false;
		normalsDesc.colorspace = ColorSpace::RGBA;
		mViewSpaceNormalsRT = std::make_unique<RenderTarget2D>(width, height, normalsDesc, 1);
	}

	BilateralBlurShader::BilateralBlurShader(unsigned kernelRadius)
	{
		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/hbao/bilateralblur.frag.glsl", nullptr, nullptr, nullptr,
			{"#define KERNEL_RADIUS " + std::to_string(kernelRadius)});

		mSharpness = { mProgram->getUniformLocation("g_Sharpness"), UniformType::FLOAT };
		mInvResolutionDirection = { mProgram->getUniformLocation("g_InvResolutionDirection"), UniformType::VEC2 };

		mAoTexture = mProgram->createTextureUniform("texSource", UniformType::TEXTURE2D, 0);
		mViewSpaceZ = mProgram->createTextureUniform("texViewSpaceZ", UniformType::TEXTURE2D, 1);
	}

	void BilateralBlurShader::draw(RenderTarget2D* temp,
		RenderTarget2D* result,
		const Texture* viewspaceZ,
		const Texture* aoUnblurred,
		float width,
		float height,
		float sharpness)
	{
		temp->bind();
		bind();

		mProgram->setTexture(aoUnblurred, Sampler::getPoint(), 0);
		mProgram->setTexture(viewspaceZ, Sampler::getPoint(), 1);

		mProgram->setFloat(mSharpness.location, sharpness);

		// blur horizontal
		mProgram->setVec2(mInvResolutionDirection.location, glm::vec2(1.0f / width, 0));

		const auto& state = RenderState::getNoDepthTest();
		Drawer::drawFullscreenTriangle(state, this);

		// blur vertically
		result->bind();
		auto* renderResult = temp->getColorAttachments()[0].texture.get();

		// Note: mSampler is already bound to binding point 0!
		mProgram->setTexture(renderResult, nullptr, 0); // TODO: check binding point!
		mProgram->setVec2(mInvResolutionDirection.location, glm::vec2(0, 1.0f / height));

		Drawer::drawFullscreenTriangle(state, this);
	}

	DepthToViewSpaceZShader::DepthToViewSpaceZShader()
	{
		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/hbao/depthlinearize.frag.glsl");
		mSampler = std::make_unique<Sampler>(SamplerDesc());
		mSampler->setMinFilter(TexFilter::Nearest);
		mSampler->setMagFilter(TexFilter::Nearest);

		mSampler->setWrapR(UVTechnique::ClampToBorder);
		mSampler->setWrapS(UVTechnique::ClampToBorder);
		mSampler->setWrapT(UVTechnique::ClampToBorder);
		mSampler->setBorderColor(glm::vec4(1.0));


		UniformLocation inputLoc = mProgram->getUniformLocation("inputTexture");
		mProgram->setBinding(inputLoc, 0);
	}

	void DepthToViewSpaceZShader::draw(const Projection& projection, const Texture* depth)
	{
		bind();

		glm::vec4 vecData(projection.nearplane * projection.farplane,
			projection.nearplane - projection.farplane,
			projection.farplane,
			projection.perspective ? 1.0f : 0.0f);
		mProgram->setVec4(0, vecData);

		mProgram->setTexture(depth, mSampler.get(), 0);
		
		const auto& state = RenderState::getNoDepthTest();
		Drawer::drawFullscreenTriangle(state, this);

		mProjection = projection;
	}

	const Projection& DepthToViewSpaceZShader::getLastProjectionData() const
	{
		return mProjection;
	}

	DisplayAoShader::DisplayAoShader() : Shader()
	{
		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/hbao/hbao_ao_display_fs.glsl");

		UniformLocation inputLoc = mProgram->getUniformLocation("inputTexture");
		mProgram->setBinding(inputLoc, 0);
	}

	void DisplayAoShader::draw(const Texture* ao)
	{
		bind();
		mProgram->setTexture(ao, Sampler::getLinear(), 0); // TODO: check binding point!
		
		const auto& state = RenderState::getNoDepthTest();
		Drawer::drawFullscreenTriangle(state, this);
	}

	HbaoShader::HbaoShader()
	{
		std::vector<std::string> defines = { "#define AO_DEINTERLEAVED 0", "#define AO_BLUR 1" };

		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/hbao/hbao_fs.glsl", //post_processing/hbao/fullscreenquad.vert.glsl
			nullptr, nullptr, nullptr, defines);

		UniformLocation linearDepthLoc = mProgram->getUniformLocation("texLinearDepth");
		mProgram->setBinding(linearDepthLoc, 0);

		UniformLocation randomLoc = mProgram->getUniformLocation("texRandom");
		mProgram->setBinding(randomLoc, 1);

		auto state = Sampler::getPoint()->getState();
		state.wrapR = state.wrapS = state.wrapT = UVTechnique::Repeat;
		mPointSampler2.setState(state);
	}

	void HbaoShader::setHbaoUBO(UniformBuffer* hbao_ubo)
	{
		hbao_ubo->bindToTarget(0);
	}

	void HbaoShader::setLinearDepth(Texture * linearDepth)
	{
		mProgram->setTexture(linearDepth, Sampler::getPoint(), 0);
	}

	void HbaoShader::setRamdomView(Texture * randomView)
	{
		mProgram->setTexture(randomView, &mPointSampler2, 1);
	}


	HbaoDeinterleavedShader::HbaoDeinterleavedShader()
	{
		std::vector<std::string> defines = { "#define AO_DEINTERLEAVED 1", "#define AO_BLUR 1" };

		mProgram = ShaderProgram::create("post_processing/hbao/hbao_deinterleaved_fullscreen_triangle_vs.glsl", "post_processing/hbao/hbao_fs.glsl",  //post_processing/hbao/fullscreenquad.vert.glsl
			nullptr, nullptr, nullptr, defines);

		//"post_processing/hbao/fullscreenquad.geo.glsl"

		mLinearDepth = mProgram->createTextureUniform("texLinearDepth", UniformType::TEXTURE2D_ARRAY, 0);
		mViewNormals = mProgram->createTextureUniform("texViewNormal", UniformType::TEXTURE2D, 1);
		mImgOutput = { mProgram->getUniformLocation("imgOutput"), UniformType::IMAGE2D_ARRAY, 0 };
	}
	void HbaoDeinterleavedShader::setHbaoUBO(UniformBuffer* hbao_ubo)
	{
		hbao_ubo->bindToTarget(0);
	}
	void HbaoDeinterleavedShader::setLinearDepth(Texture* linearDepth)
	{
		mProgram->setTexture(linearDepth, Sampler::getPoint(), mLinearDepth.bindingSlot);
	}
	void HbaoDeinterleavedShader::setViewNormals(Texture* normals)
	{
		mProgram->setTexture(normals, Sampler::getPoint(), mViewNormals.bindingSlot);
	}

	void HbaoDeinterleavedShader::setImageOutput(Texture* imgOutput)
	{
		mProgram->setImageLayerOfTexture(mImgOutput.location, 
			imgOutput, 
			mImgOutput.location, 
			TextureAccess::WRITE_ONLY, 
			InternalFormat::RG16F, 
			0, 
			true, 0);
	}




	ViewNormalShader::ViewNormalShader()
	{
		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/hbao/viewnormal_fs.glsl");
		mProjInfo = {mProgram->getUniformLocation("projInfo"), UniformType::VEC4};
		mProjOrtho = { mProgram->getUniformLocation("projOrtho"), UniformType::INT };
		mInvFullResolution = { mProgram->getUniformLocation("InvFullResolution"), UniformType::VEC2 };
		mLinearDepth = mProgram->createTextureUniform("texLinearDepth", UniformType::TEXTURE2D, 0);
	}

	ViewNormalShader::~ViewNormalShader() = default;

	void ViewNormalShader::setProjInfo(const glm::vec4 & projInfo)
	{
		mProgram->setVec4(mProjInfo.location, projInfo);
	}
	void ViewNormalShader::setProjOrtho(bool projOrtho)
	{
		mProgram->setInt(mProjOrtho.location, static_cast<int>(projOrtho));
	}
	void ViewNormalShader::setInvFullResolution(const glm::vec2 & invFullResolution)
	{
		mProgram->setVec2(mInvFullResolution.location, invFullResolution);
	}
	void ViewNormalShader::setLinearDepth(Texture * linearDepth)
	{
		mProgram->setTexture(linearDepth, Sampler::getLinear(), mLinearDepth.bindingSlot);
	}


	DeinterleaveShader::DeinterleaveShader()
	{
		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/hbao/hbao_deinterleave_fs.glsl");
		mInfo = { mProgram->getUniformLocation("info"), UniformType::VEC4 };
		mLinearDepth = mProgram->createTextureUniform("texLinearDepth", UniformType::TEXTURE2D, 0);
	}
	DeinterleaveShader::~DeinterleaveShader() = default;

	void DeinterleaveShader::setInfo(const glm::vec4 & info)
	{
		mProgram->setVec4(mInfo.location, info);
	}

	void DeinterleaveShader::setLinearDepth(Texture* linearDepth)
	{
		mProgram->setTexture(linearDepth, Sampler::getLinear(), mLinearDepth.bindingSlot);
	}

	ReinterleaveShader::ReinterleaveShader()
	{
		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/hbao/hbao_reinterleave_fs.glsl",
			nullptr, nullptr, nullptr, {"#define AO_BLUR 1"});
		mResultArray = mProgram->createTextureUniform("texResultsArray", UniformType::TEXTURE2D_ARRAY, 0);
	}

	ReinterleaveShader::~ReinterleaveShader() = default;

	void ReinterleaveShader::setTextureResultArray(Texture * resultArray)
	{
		mProgram->setTexture(resultArray, Sampler::getPoint(), mResultArray.bindingSlot);
	}

	HbaoBlur::HbaoBlur(unsigned kernelRadius) : mActivePreset(0)
	{
		mBlurPreset[0] = std::make_unique<Shader>(ShaderProgram::create("screen_space_vs.glsl", 
			"post_processing/hbao/hbao_blur_fs.glsl",
			nullptr, nullptr, nullptr, {"#define AO_BLUR_PRESENT 0", "#define KERNEL_RADIUS " + std::to_string(kernelRadius)}));

		mBlurPreset[1] = std::make_unique<Shader>(ShaderProgram::create("screen_space_vs.glsl",
			"post_processing/hbao/hbao_blur_fs.glsl",
			nullptr, nullptr, nullptr, { "#define AO_BLUR_PRESENT 1", "#define KERNEL_RADIUS " + std::to_string(kernelRadius) }));

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
		mBlurPreset[mActivePreset]->bind();
	}

	nex::Shader* HbaoBlur::getPreset(int id)
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

	void HbaoBlur::setSource(const Texture* source)
	{
		mBlurPreset[mActivePreset]->getShader()->setTexture(source, Sampler::getPoint(), mSource.bindingSlot);
	}

	void HbaoBlur::draw(RenderTarget2D* temp, RenderTarget2D* result, const Texture* aoUnblurred, float width, float height, float sharpness)
	{
		const auto& state = RenderState::getNoDepthTest();
		temp->bind();
		bindPreset(0);
		setSource(aoUnblurred);
		setSharpness(sharpness);
		setInvResolutionDirection({ 1.0f / width, 0.0f });
		Drawer::drawFullscreenTriangle(state, getPreset(0));

		result->bind();
		bindPreset(1);
		setSource(temp->getColorAttachments()[0].texture.get());
		setSharpness(sharpness);
		setInvResolutionDirection({ 0.0f, 1.0f / height });
		Drawer::drawFullscreenTriangle(state, getPreset(1));
	}


	float HBAO::getBlurSharpness() const
	{
		return mBlurSharpness;
	}

	unsigned HBAO::getBlurKernelRadius() const
	{
		return mBlurKernelRadius;
	}

	void HBAO::setBlurSharpness(float sharpness)
	{
		mBlurSharpness = sharpness;
	}

	void HBAO::setBlurKernelRadius(unsigned radius)
	{
		if (mBlurKernelRadius != radius) {
			mBlurKernelRadius = radius;
			reloadBlurShaders(radius);
		}
	}

	void HBAO::useSpecialBlur()
	{
		mUseSpecialBlur = true;
	}

	void HBAO::useBilaterialBlur()
	{
		mUseSpecialBlur = false;
	}

	void HBAO::useBlur(bool doBlur)
	{
		mBlurAo = doBlur;
	}

	void HBAO::useDeinterleavedTexturing(bool useDeinterleaved)
	{
		mUseDeinterleavedTexturing = useDeinterleaved;
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

	HbaoConfigurationView::HbaoConfigurationView(HBAO * hbao) : mHbao(hbao),
		mApplyButton([this]() {mHbao->setBlurKernelRadius(mBlurKernelRadius); },
			[this]() {mBlurKernelRadius = mHbao->getBlurKernelRadius(); })
	{
		mIsVisible = true;
		mBlurKernelRadius = mHbao->getBlurKernelRadius();
	}

	void HbaoConfigurationView::drawSelf()
	{
		// render configuration properties
		ImGui::PushID(mId.c_str());
		ImGui::LabelText("", "HBAO:");
		ImGui::SliderFloat("Bias", &mHbao->mBias, 0.0f, 5.0f);
		ImGui::SliderFloat("Blur sharpness", &mHbao->mBlurSharpness, 0.0f, 1000.0f);
		ImGui::SliderFloat("Intensity", &mHbao->mIntensity, 0.0f, 10.0f);
		ImGui::SliderFloat("Meters to viewspace unit transformation", &mHbao->mMeters2ViewSpace, 0.0f, 10.0f);
		ImGui::SliderFloat("Radius", &mHbao->mRadius, 0.0f, 10.0f);
		ImGui::Checkbox("Blur", &mHbao->mBlurAo);

		if (mHbao->mBlurAo) {
			
			static const char* blurTechniques[] = {"Bilaterial", "Optimized bilaterial"};
			int current = mHbao->mUseSpecialBlur ? 1 : 0;

			if (ImGui::Combo("Blur technique", &current, blurTechniques, 2))
				mHbao->mUseSpecialBlur = current == 1;

			if (ImGui::SliderInt("Blur kernel radius", &mBlurKernelRadius, 1, 12)) {

			}



			if (mBlurKernelRadius != mHbao->getBlurKernelRadius()) {
				mApplyButton.drawGUI();
			}
		}

		ImGui::Checkbox("Use deinterleaved texturing", &mHbao->mUseDeinterleavedTexturing);


		//ImGui::Dummy(ImVec2(100, 200));
		ImGui::Dummy(ImVec2(0, 20));
		nex::gui::Separator(2.0f);
		//ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
		//	ImVec2(100.f, 120.f), ImColor(255, 255, 0, 255), "Hello World##HelloWorld!", 0, 0.0f, 0);

		ImGui::PopID();
	}
}