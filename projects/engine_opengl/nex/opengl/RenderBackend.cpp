﻿#include "nex/renderer/RenderBackend.hpp"
#include <nex/opengl/RenderBackendGL.hpp>
#include <nex/texture/TextureManager.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <nex/opengl/opengl.hpp>
#include <nex/renderer/Drawer.hpp>
#include "nex/texture/Attachment.hpp"
#include "nex/effects/SkyBoxPass.hpp"
#include <nex/effects/EffectLibrary.hpp>
#include "CacheGL.hpp"
#include "nex/resource/ResourceLoader.hpp"

using namespace std;
using namespace nex;
using namespace glm;

namespace nex
{
	BlendDescGL::BlendDescGL(const BlendDesc& desc) :
		source(translate(desc.source)),
		destination(translate(desc.destination)),
		operation(translate(desc.operation))
	{
	}

	RenderTargetBlendDescGL::RenderTargetBlendDescGL() : RenderTargetBlendDescGL(RenderTargetBlendDesc())
	{
	}

	RenderTargetBlendDescGL::RenderTargetBlendDescGL(const RenderTargetBlendDesc& desc) :
		enableBlend(translate(desc.enableBlend)),
		colorAttachIndex(desc.colorAttachIndex),
		blendDesc(desc.blendDesc)
	{

	}

	Blender::Blender()
	{
		mImpl = make_unique<Blender::Impl>();
		setState(BlendState());
	}

	Blender::Impl::Impl() :
		mBlendDesc(BlendDesc())
	{
		glGetBooleanv(GL_BLEND, &mEnableBlend);
		glGetBooleanv(GL_SAMPLE_COVERAGE, (GLboolean*)&mEnableAlphaToCoverage);
	}

	void Blender::enableBlend(bool enable)
	{
		if (mImpl->mEnableBlend == enable) return;

		mImpl->mEnableBlend = enable;

		if (enable)
		{
			GLCall(glEnable(GL_BLEND));
		}
		else
		{
			GLCall(glDisable(GL_BLEND));
		}
	}

	void Blender::enableAlphaToCoverage(bool enable)
	{
		if (mImpl->mEnableAlphaToCoverage == enable) return;
		mImpl->mEnableAlphaToCoverage = enable;

		if (enable)
		{
			GLCall(glEnable(GL_SAMPLE_COVERAGE));
		}
		else
		{
			GLCall(glDisable(GL_SAMPLE_COVERAGE));
		}
	}

	void Blender::setSampleConverage(float sampleCoverage, bool invert)
	{
		mImpl->mSampleCoverage = sampleCoverage;
		mImpl->mInvertSampleConverage = translate(invert);
		GLCall(glSampleCoverage(mImpl->mSampleCoverage, mImpl->mInvertSampleConverage));
	}

	void Blender::setConstantBlendColor(const glm::vec4& color)
	{
		mImpl->mConstantBlendColor = color;
		GLCall(glBlendColor(color.r, color.g, color.b, color.a));
	}

	void Blender::setBlendDesc(const BlendDesc& desc)
	{
		// Translate description
		mImpl->mBlendDesc = desc;

		GLCall(glBlendEquation((GLenum)mImpl->mBlendDesc.operation));
		GLCall(glBlendFunc((GLenum)mImpl->mBlendDesc.source, (GLenum)mImpl->mBlendDesc.destination));
	}

	void Blender::setState(const BlendState& state)
	{
		enableBlend(state.enableBlend);
		enableAlphaToCoverage(state.enableAlphaToCoverage);
		setSampleConverage(state.sampleCoverage, state.invertSampleConverage);
		setConstantBlendColor(state.constantBlendColor);
		setBlendDesc(state.globalBlendDesc);
	}

	void Blender::setRenderTargetBlending(const RenderTargetBlendDesc & blendDesc)
	{
		RenderTargetBlendDescGL descGL(blendDesc);
		mImpl->mRenderTargetBlendings[blendDesc.colorAttachIndex] = descGL;

		if (blendDesc.enableBlend)
		{
			GLCall(glEnablei(GL_BLEND, descGL.colorAttachIndex));
			GLCall(glBlendEquationi(descGL.colorAttachIndex, (GLenum)descGL.blendDesc.operation));
			GLCall(glBlendFunci(descGL.colorAttachIndex,
				(GLenum)descGL.blendDesc.source, (GLenum)descGL.blendDesc.destination));
		}
		else
		{
			GLCall(glDisablei(GL_BLEND, descGL.colorAttachIndex));
		}
	}

	DepthBuffer::DepthBuffer() : mImpl(std::make_unique<DepthBuffer::Impl>())
	{
		setState(DepthBuffer::State());
	}

	DepthBuffer::Impl::Impl()
	{
		glGetBooleanv(GL_DEPTH_TEST, &mEnableDepthTest);
		glGetBooleanv(GL_DEPTH_CLAMP, &mEnableDepthClamp);
		glGetBooleanv(GL_DEPTH_WRITEMASK, &mEnableDepthBufferWriting);
		glGetIntegerv(GL_DEPTH_FUNC, (GLint*)&mDepthFunc);
	}

	void DepthBuffer::enableDepthBufferWriting(bool enable)
	{
		//if (mImpl->mEnableDepthBufferWriting == enable) return;

		mImpl->mEnableDepthBufferWriting = enable;
		GLCall(glDepthMask(translate(mImpl->mEnableDepthBufferWriting)));
	}

	void DepthBuffer::enableDepthTest(bool enable)
	{
		//if (mImpl->mEnableDepthTest == enable) return;

		mImpl->mEnableDepthTest = enable;

		if (enable)
		{
			GLCall(glEnable(GL_DEPTH_TEST));
		}
		else
		{
			GLCall(glDisable(GL_DEPTH_TEST));
		}
	}

	void DepthBuffer::enableDepthClamp(bool enable)
	{
		//if (mImpl->mEnableDepthClamp == enable) return;

		mImpl->mEnableDepthClamp = enable;

		if (enable)
		{
			GLCall(glEnable(GL_DEPTH_CLAMP));
		}
		else
		{
			GLCall(glDisable(GL_DEPTH_CLAMP));
		}
	}

	void DepthBuffer::setDefaultDepthFunc(CompFunc depthFunc)
	{
		const auto translated = translate(depthFunc);

		//if (mImpl->mDepthFunc == translated) return;

		mImpl->mDepthFunc = translated;
		GLCall(glDepthFunc((GLenum)mImpl->mDepthFunc));
	}

	void DepthBuffer::setDepthRange(const Range& range)
	{
		mImpl->mDepthRange = range;
		GLCall(glDepthRange(range.nearVal, range.farVal));
	}

	void DepthBuffer::setState(const State& state)
	{
		enableDepthBufferWriting(state.enableDepthBufferWriting);
		enableDepthTest(state.enableDepthTest);
		enableDepthClamp(state.enableDepthClamp);
		setDefaultDepthFunc(state.depthFunc);
		setDepthRange(state.depthRange);
	}

	Rasterizer::Rasterizer()
	{
		mImpl = make_unique<Impl>();
		setState(RasterizerState());
	}

	Rasterizer::Impl::Impl()
	{
		glGetIntegerv(GL_CULL_FACE_MODE, (GLint*)&mCullMode);
		glGetBooleanv(GL_CULL_FACE, (GLboolean*)&mEnableFaceCulling);
		glGetBooleanv(GL_SCISSOR_TEST, (GLboolean*)&mEnableScissorTest);
		glGetBooleanv(GL_MULTISAMPLE, (GLboolean*)&mEnableMultisample);
		glGetBooleanv(GL_POLYGON_OFFSET_FILL, (GLboolean*)&mEnableOffsetPolygonFill);
		glGetBooleanv(GL_POLYGON_OFFSET_LINE, (GLboolean*)&mEnableOffsetLine);
		glGetBooleanv(GL_POLYGON_OFFSET_POINT, (GLboolean*)&mEnableOffsetPoint);
	}

	void nex::Rasterizer::setFillMode(FillMode fillMode)
	{
		const auto fillModeGL = translate(fillMode);

		if (mImpl->mFillModeCache.mode == fillModeGL) return;

		mImpl->mFillModeCache.mode = fillModeGL;
		mImpl->mFillModeCache.side = PolygonSideGL::FRONT_BACK;
		GLCall(glPolygonMode((GLenum)PolygonSideGL::FRONT_BACK, (GLenum)fillModeGL));
	}

	void nex::Rasterizer::setCullMode(PolygonSide faceSide)
	{
		const auto translated = translate(faceSide);
		if (mImpl->mCullMode == translated) return;
		mImpl->mCullMode = translated;
		GLCall(glCullFace((GLenum)mImpl->mCullMode));
	}

	void Rasterizer::setWindingOrder(WindingOrder order)
	{
		mImpl->mWindingOrder = translate(order);
		GLCall(glFrontFace((GLenum)mImpl->mWindingOrder));
	}

	void nex::Rasterizer::setDepthBias(float slopeScale, float unit, float clamp)
	{
		mImpl->mDepthBias = unit;
		mImpl->mSlopeScaledDepthBias = slopeScale;
		mImpl->mDepthBiasClamp = clamp;

		//TODO use clamp with EXT_polygon_offset_clamp !
		GLCall(glPolygonOffset(slopeScale, unit));
	}

	void nex::Rasterizer::setState(const RasterizerState& state)
	{
		setFillMode(state.fillMode);
		setCullMode(state.cullMode);
		setWindingOrder(state.windingOrder);
		setDepthBias(state.slopeScaledDepthBias, state.depthBias, state.depthBiasClamp);
		enableFaceCulling(state.enableFaceCulling);
		enableScissorTest(state.enableScissorTest);
		enableMultisample(state.enableMultisample);
		enableOffsetPolygonFill(state.enableOffsetPolygonFill);
		enableOffsetLine(state.enableOffsetLine);
		enableOffsetPoint(state.enableOffsetPoint);
	}

	void nex::Rasterizer::enableFaceCulling(bool enable)
	{
		if (mImpl->mEnableFaceCulling == enable) return;

		mImpl->mEnableFaceCulling = enable;
		if (enable)
		{
			GLCall(glEnable(GL_CULL_FACE));
		}
		else
		{
			GLCall(glDisable(GL_CULL_FACE));
		}
	}

	void nex::Rasterizer::enableScissorTest(bool enable)
	{
		if (mImpl->mEnableScissorTest == enable) return;
		mImpl->mEnableScissorTest = enable;

		if (enable)
		{
			GLCall(glEnable(GL_SCISSOR_TEST));
		}
		else
		{
			GLCall(glDisable(GL_SCISSOR_TEST));
		}
	}

	void nex::Rasterizer::enableMultisample(bool enable)
	{
		if (mImpl->mEnableMultisample == enable) return;
		mImpl->mEnableMultisample = enable;

		if (enable)
		{
			GLCall(glEnable(GL_MULTISAMPLE));
		}
		else
		{
			GLCall(glDisable(GL_MULTISAMPLE));
		}
	}

	void nex::Rasterizer::enableOffsetPolygonFill(bool enable)
	{
		if (mImpl->mEnableOffsetPolygonFill == enable) return;
		mImpl->mEnableOffsetPolygonFill = enable;

		if (enable)
		{
			GLCall(glEnable(GL_POLYGON_OFFSET_FILL));
		}
		else
		{
			GLCall(glDisable(GL_POLYGON_OFFSET_FILL));
		}
	}

	void nex::Rasterizer::enableOffsetLine(bool enable)
	{
		if (mImpl->mEnableOffsetLine == enable) return;
		mImpl->mEnableOffsetLine = enable;

		if (enable)
		{
			GLCall(glEnable(GL_POLYGON_OFFSET_LINE));
		}
		else
		{
			GLCall(glDisable(GL_POLYGON_OFFSET_LINE));
		}
	}

	void nex::Rasterizer::enableOffsetPoint(bool enable)
	{
		if (mImpl->mEnableOffsetPoint == enable) return;
		mImpl->mEnableOffsetPoint = enable;

		if (enable)
		{
			GLCall(glEnable(GL_POLYGON_OFFSET_POINT));
		}
		else
		{
			GLCall(glDisable(GL_POLYGON_OFFSET_POINT));
		}
	}

	StencilTest::StencilTest()
	{
		mImpl = make_unique<Impl>();
		setState(State());
	}

	StencilTest::Impl::Impl()
	{
		glGetBooleanv(GL_STENCIL_TEST, (GLboolean*)&mEnableStencilTest);
		glGetIntegerv(GL_STENCIL_FUNC, (GLint*)&mCompareFunc);
		glGetIntegerv(GL_STENCIL_FUNC, (GLint*)&mCompareReferenceValue);
		glGetIntegerv(GL_STENCIL_FUNC, (GLint*)&mCompareMask);

		glGetIntegerv(GL_STENCIL_FAIL, (GLint*)&mStencilTestFailOperation);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, (GLint*)&mDepthTestFailOperation);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, (GLint*)&mDepthPassOperation);
	}

	void StencilTest::enableStencilTest(bool enable)
	{
		if (mImpl->mEnableStencilTest == enable) return;
		mImpl->mEnableStencilTest = enable;

		if (mImpl->mEnableStencilTest)
		{
			GLCall(glEnable(GL_STENCIL_TEST));
		}
		else
		{
			GLCall(glDisable(GL_STENCIL_TEST));
		}
	}

	void StencilTest::setCompareFunc(CompFunc func, int referenceValue, unsigned mask)
	{
		const auto translated = translate(func);
		if (mImpl->mCompareFunc == translated && mImpl->mCompareReferenceValue == referenceValue && mImpl->mCompareMask == mask) return;


		mImpl->mCompareFunc = translated;
		mImpl->mCompareReferenceValue = referenceValue;
		mImpl->mCompareMask = mask;

		GLCall(glStencilFunc((GLenum)mImpl->mCompareFunc, mImpl->mCompareReferenceValue, mImpl->mCompareMask));
	}

	void StencilTest::setOperations(Operation stencilFail, Operation depthFail, Operation depthPass)
	{
		const auto stencilFailTrans = translate(stencilFail);
		const auto depthFailTrans = translate(depthFail);
		const auto depthPassTrans = translate(depthPass);

		if (mImpl->mStencilTestFailOperation == stencilFailTrans
			&& mImpl->mDepthTestFailOperation == depthFailTrans
			&& mImpl->mDepthPassOperation == depthPassTrans) return;


		mImpl->mStencilTestFailOperation = stencilFailTrans;
		mImpl->mDepthTestFailOperation = depthFailTrans;
		mImpl->mDepthPassOperation = depthPassTrans;

		GLCall(glStencilOp(mImpl->mStencilTestFailOperation, mImpl->mDepthTestFailOperation, mImpl->mDepthPassOperation));
	}

	void StencilTest::setState(const State& state)
	{
		enableStencilTest(state.enableStencilTest);
		setCompareFunc(state.compareFunc, state.compareReferenceValue, state.compareMask);
		setOperations(state.stencilTestFailOperation, state.depthTestFailOperation, state.depthPassOperation);
	}

	RenderBackend::Impl::Impl(): m_logger("RenderBackend - OPENGL"),
	                             backgroundColor(0.0f, 0.0f, 0.0f),
	                             msaaSamples(1), defaultRenderTarget(nullptr)
	{
		bool test = false;
	}

	RenderBackend::Impl::~Impl() {
		GLDeactivateLog();
	};

	RenderBackend::RenderBackend()
	{
		//__clearRenderTarget(&singleSampledScreenBuffer, false);
		//__clearRenderTarget(&multiSampledScreenBuffer, false);
	}

	RenderBackend::~RenderBackend()
	{
		//delete mPimpl;
		//mPimpl = nullptr;
	}

	void RenderBackend::init(const Rectangle& viewport, unsigned msaaSamples)
	{
		mPimpl = std::make_unique<Impl>();

		mPimpl->mViewport = viewport;
		mPimpl->msaaSamples = msaaSamples;

		LOG(mPimpl->m_logger, Info) << "Initializing...";
		//checkGLErrors(BOOST_CURRENT_FUNCTION);

		getRasterizer()->enableScissorTest(true);
		setViewPort(0, 0, mPimpl->mViewport.width, mPimpl->mViewport.height);
		setScissor(0, 0, mPimpl->mViewport.width, mPimpl->mViewport.height);
		mPimpl->defaultRenderTarget = make_unique<RenderTarget2D>(make_unique<RenderTarget::Impl>(GL_FALSE, mPimpl->mViewport.width, mPimpl->mViewport.height));
		mPimpl->defaultRenderTarget->bind();
		mPimpl->defaultRenderTarget->clear(RenderComponent::Color);
		GLCall(glClearColor(0.0, 0.0, 0.0, 1.0)); // TODO abstract

		getDepthBuffer()->enableDepthTest(true);
		getDepthBuffer()->setDefaultDepthFunc(CompFunc::LESS);

		getDepthBuffer()->enableDepthBufferWriting(true);


		//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		GLCall(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS)); // TODO abstract?

		// we want counter clock wise winding order
		getRasterizer()->setWindingOrder(WindingOrder::COUNTER_CLOCKWISE);
		getStencilTest()->enableStencilTest(true);
		getRasterizer()->enableFaceCulling(true);
		getRasterizer()->setCullMode(PolygonSide::BACK);

		//GLCall(glClearColor(0.0, 0.0, 0.0, 1.0));
		GLCall(glClearDepth(1.0f));
		GLCall(glClearStencil(0)); // TODO abstract?
		GLCall(glStencilMask(0xFF)); // TODO abstract ?
		getStencilTest()->setCompareFunc(CompFunc::LESS, 0, 0xFF); // TODO: Is it right?


		
		//GLCall(glEnable(GL_LINE_SMOOTH));
		//GLCall(glEnable(GL_POLYGON_SMOOTH));
		//GLCall(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
		//GLCall(glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST));

		getBlender()->enableBlend(true);

		setPointThickness(3.0f);


		mPimpl->mScreenSprite.setPosition({ 0,0 });

		//checkGLErrors(BOOST_CURRENT_FUNCTION);


		


		GLCall(glClipControl(EUCLID_OPENGL_SCREEN_SPACE_ORIGIN, EUCLID_OPENGL_CLIP_RANGE));
	


	}

	void RenderBackend::initEffectLibrary()
	{
		mPimpl->mEffectLibrary = make_unique<EffectLibrary>(mPimpl->mViewport.width, mPimpl->mViewport.height);	
	}

	std::unique_ptr<CubeDepthMap> RenderBackend::createCubeDepthMap(int width, int height)
	{
		return std::unique_ptr<CubeDepthMap>(CubeDepthMap::create(width, height));
	}

	std::unique_ptr<CubeRenderTarget> nex::RenderBackend::createCubeRenderTarget(int width, int height, const TextureDesc& data)
	{
		return make_unique<CubeRenderTarget>(width, height, data);
	}

	RenderTarget2D* nex::RenderBackend::getDefaultRenderTarget()
	{
		return mPimpl->defaultRenderTarget.get();
	}

	DepthBuffer* RenderBackend::getDepthBuffer()
	{
		return &mPimpl->mDepthBuffer;
	}

	std::unique_ptr <RenderTarget2D> nex::RenderBackend::create2DRenderTarget(int width, int height, const TextureDesc& data, const TextureDesc& depthData, int samples) {
		RenderAttachment depth;
		depth.type = RenderAttachment::translate(depthData.internalFormat);

		if (samples > 1) {
			depth.texture = make_shared<Texture2DMultisample>(width, height, depthData, samples); //Texture2DMultisample
		}
		else {
			depth.texture = make_shared<Texture2D>(width, height, depthData, nullptr);
		}

		auto result = make_unique<RenderTarget2D>(width, height, data, samples);
		result->useDepthAttachment(std::move(depth));
		result->finalizeAttachments();
		return result;
	}

	std::unique_ptr <RenderTarget2D> nex::RenderBackend::createRenderTarget(int samples)
	{
		const int ssaaSamples = 1;

		const unsigned width = mPimpl->mViewport.width * ssaaSamples;
		const unsigned height = mPimpl->mViewport.height * ssaaSamples;

		TextureDesc depthData = TextureDesc::createDepth(CompFunc::LESS, InternalFormat::DEPTH32F_STENCIL8); //DEPTH32F_STENCIL8

		return create2DRenderTarget(width, height, TextureDesc::createRenderTargetRGBAHDR(), depthData, samples);
	}

	void RenderBackend::drawWithIndices(const RenderState& state, Topology topology, size_t indexCount, IndexElementType indexType, size_t byteOffset)
	{
		setRenderState(state);
		GLCall(glDrawElements((GLenum)translate(topology), static_cast<GLsizei>(indexCount), (GLenum)translate(indexType), (GLvoid*)byteOffset));
	}

	void RenderBackend::drawWithIndicesInstanced(	size_t instanceCount, 
													const RenderState& state, 
													Topology topology, 
													size_t indexCount, 
													IndexElementType indexType, 
													size_t byteOffset)
	{
		setRenderState(state);
		GLCall(glDrawElementsInstanced(	(GLenum)translate(topology), 
										static_cast<GLsizei>(indexCount), 
										(GLenum)translate(indexType), 
										(GLvoid*)byteOffset, 
										static_cast<GLsizei>(instanceCount)));
	}

	void RenderBackend::flushPendingCommands()
	{
		GLCall(glFlush());
	}

	RenderBackend* RenderBackend::get()
	{
		static thread_local  RenderBackend backend;
		return &backend;
	}

	Blender * RenderBackend::getBlender()
	{
		return &mPimpl->mBlender;
	}

	unsigned RenderBackend::getMaxPatchVertexCount() const
	{
		return GlobalCacheGL::get()->GetConstInteger(GL_MAX_PATCH_VERTICES);
	}

	Rasterizer * RenderBackend::getRasterizer()
	{
		return &mPimpl->mRasterizer;
	}

	nex::Sprite* RenderBackend::getScreenSprite()
	{
		return &mPimpl->mScreenSprite;
	}

	StencilTest* RenderBackend::getStencilTest()
	{
		return &mPimpl->mStencilTest;
	}

	RendererType RenderBackend::getType() const
	{
		return OPENGL;
	}

	const Rectangle& RenderBackend::getViewport() const
	{
		return mPimpl->mViewport;
	}

	void RenderBackend::resize(int width, int height)
	{
		mPimpl->mViewport.width = width;
		mPimpl->mViewport.height = height;
		mPimpl->defaultRenderTarget = make_unique<RenderTarget2D>(make_unique<RenderTarget::Impl>(GL_FALSE, mPimpl->mViewport.width, mPimpl->mViewport.height));
		setViewPort(0, 0, width, height);
		mPimpl->mEffectLibrary->resize(width, height);

		mPimpl->mScreenSprite.setWidth(width);
		mPimpl->mScreenSprite.setHeight(height);	
	}

	void RenderBackend::release()
	{
		mPimpl.reset(nullptr);
		//mPimpl->mEffectLibrary.reset(nullptr);
	}

	void RenderBackend::syncMemoryWithGPU(int flags)
	{
		GLuint glFlags = 0;
		if (flags & MemorySync_ShaderImageAccess)
			glFlags |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
		if (flags & MemorySync_ShaderStorage)
			glFlags |= GL_SHADER_STORAGE_BARRIER_BIT;
		if (flags |= MemorySync_TextureFetch)
			glFlags |= GL_TEXTURE_FETCH_BARRIER_BIT;
		if (flags & MemorySync_TextureUpdate)
			glFlags |= GL_TEXTURE_UPDATE_BARRIER_BIT;

		GLCall(glMemoryBarrier(glFlags));
	}

	void RenderBackend::setBackgroundColor(const glm::vec3& color)
	{
		mPimpl->backgroundColor = color;
		GLCall(glClearColor(color.r, color.g, color.b, 1.0f));
	}

	void RenderBackend::setLineThickness(float thickness)
	{
		assert(thickness >= 0.0f);
		GLCall(glLineWidth(thickness));
	}

	void RenderBackend::setPointThickness(float thickness)
	{
		assert(thickness >= 0.0f);
		GLCall(glPointSize(thickness));
	}

	void RenderBackend::setMSAASamples(unsigned int samples)
	{
		if (samples == 0)
		{
			// Samples smaller one cannot be handled by OpenGL
			mPimpl->msaaSamples = 1;
		}
		else
		{
			mPimpl->msaaSamples = samples;
		}
	}

	void RenderBackend::setPatchVertexCount(unsigned number)
	{
		assert(number >= 3 && number <= getMaxPatchVertexCount());
		GLCall(glPatchParameteri(GL_PATCH_VERTICES, number));
	}

	void RenderBackend::setScissor(int x, int y, unsigned width, unsigned height)
	{
		GLCall(glScissor(x, y, width, height));
	}

	void RenderBackend::setViewPort(int x, int y, int width, int height)
	{
		mPimpl->mViewport.x = x;
		mPimpl->mViewport.y = y;
		mPimpl->mViewport.width = width;
		mPimpl->mViewport.height = height;

		GLCall(glViewport(x, y, width, height));
	}

	void RenderBackend::setRenderState(const RenderState& state)
	{
		auto* blender = getBlender();
		auto* rasterizer = getRasterizer();
		auto* depthBuffer = getDepthBuffer();

		blender->enableBlend(state.doBlend);
		blender->setBlendDesc(state.blendDesc);

		depthBuffer->enableDepthTest(state.doDepthTest);
		depthBuffer->enableDepthBufferWriting(state.doDepthWrite);
		depthBuffer->setDefaultDepthFunc(state.depthCompare);

		rasterizer->enableFaceCulling(state.doCullFaces);
		rasterizer->setCullMode(state.cullSide);
		rasterizer->setWindingOrder(state.windingOrder);
		rasterizer->setFillMode(state.fillMode);
		
	}

	void RenderBackend::wait()
	{
		GLCall(glFinish());
	}

	EffectLibrary* RenderBackend::getEffectLibrary()
	{
		return mPimpl->mEffectLibrary.get();
	}

	void RenderBackend::drawArray(const RenderState& state, Topology primitiveType, size_t startingIndex,
		size_t indexCount)
	{
		setRenderState(state);

		const auto primitiveTypeGL = translate(primitiveType);
		GLCall(glDrawArrays((GLenum)primitiveTypeGL, static_cast<unsigned>(startingIndex), static_cast<unsigned>(indexCount)));
	}

	void RenderBackend::drawArrayInstanced(const RenderState& state, Topology primitiveType, size_t startingIndex,
		size_t indexCount, size_t instanceCount)
	{
		setRenderState(state);

		const auto primitiveTypeGL = translate(primitiveType);
		GLCall(glDrawArraysInstanced((GLenum)primitiveTypeGL, static_cast<unsigned>(startingIndex), static_cast<unsigned>(indexCount),
			static_cast<unsigned>(instanceCount)));
	}




	TopologyGL translate(Topology topology)
	{
		static TopologyGL table[]
		{
			TopologyGL::LINES,
			TopologyGL::LINES_ADJACENCY,
			TopologyGL::LINE_LOOP,
			TopologyGL::LINE_STRIP,
			TopologyGL::LINE_STRIP_ADJACENCY,
			TopologyGL::PATCHES,
			TopologyGL::POINTS,
			TopologyGL::TRIANGLES,
			TopologyGL::TRIANGLES_ADJACENCY,
			TopologyGL::TRIANGLE_FAN,
			TopologyGL::TRIANGLE_STRIP,
			TopologyGL::TRIANGLE_STRIP_ADJACENCY,
		};

		static const unsigned size = (unsigned)Topology::LAST - (unsigned)Topology::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: Topology and TopologyGL don't match!");

		return table[(unsigned)topology];
	}

	BlendFuncGL translate(BlendFunc func)
	{
		static BlendFuncGL table[]
		{
			BlendFuncGL::ZERO,
			BlendFuncGL::ONE,

			BlendFuncGL::SOURCE_COLOR,
			BlendFuncGL::ONE_MINUS_SOURCE_COLOR,
			BlendFuncGL::DESTINATION_COLOR,
			BlendFuncGL::ONE_MINUS_DESTINATION_COLOR,

			BlendFuncGL::SOURCE_ALPHA,
			BlendFuncGL::ONE_MINUS_SOURCE_ALPHA,
			BlendFuncGL::DESTINATION_ALPHA,
			BlendFuncGL::ONE_MINUS_DESTINATION_ALPHA,

			BlendFuncGL::CONSTANT_COLOR,
			BlendFuncGL::ONE_MINUS_CONSTANT_COLOR,
			BlendFuncGL::CONSTANT_ALPHA,
			BlendFuncGL::ONE_MINUS_CONSTANT_ALPHA,
		};

		static const unsigned size = (unsigned)BlendFunc::LAST - (unsigned)BlendFunc::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: BlendFunc and BlendFuncGL don't match!");

		return table[(unsigned)func];
	}

	BlendOperationGL translate(BlendOperation op)
	{
		static BlendOperationGL table[]
		{
			BlendOperationGL::ADD,
			BlendOperationGL::SUBTRACT,
			BlendOperationGL::REV_SUBTRACT,
			BlendOperationGL::MIN,
			BlendOperationGL::MAX,
		};

		static const unsigned size = (unsigned)BlendOperation::LAST - (unsigned)BlendOperation::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: BlendFunc and BlendFuncGL don't match!");

		return table[(unsigned)op];
	}

	StencilTest::Impl::OperationGL translate(StencilTest::Operation op)
	{
		static StencilTest::Impl::OperationGL table[]
		{
			StencilTest::Impl::KEEP,
			StencilTest::Impl::ZERO,
			StencilTest::Impl::REPLACE,
			StencilTest::Impl::INCREMENT,
			StencilTest::Impl::INCREMENT_WRAP,
			StencilTest::Impl::DECREMENT,
			StencilTest::Impl::DECREMENT_WRAP,
			StencilTest::Impl::INVERT,
		};

		static const unsigned size = (unsigned)StencilTest::Operation::LAST - (unsigned)StencilTest::Operation::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: StencilTest::Operation and StencilTestGL::OperationGL don't match!");

		return table[(unsigned)op];
	}

	GLuint translate(bool boolean)
	{
		return boolean ? GL_TRUE : GL_FALSE;
	}

	nex::CompareFunctionGL translate(nex::CompFunc compareFunc)
	{
		static CompareFunctionGL const typeTable[]
		{
			CompareFunctionGL::ALWAYS,
			CompareFunctionGL::EQUAL,
			CompareFunctionGL::GREATER,
			CompareFunctionGL::GREATER_EQUAL,
			CompareFunctionGL::LESS,
			CompareFunctionGL::LESS_EQUAL,
			CompareFunctionGL::NEVER,
			CompareFunctionGL::NOT_EQUAL,
		};

		static const unsigned size = (unsigned)CompFunc::LAST - (unsigned)CompFunc::FIRST + 1;
		static_assert(sizeof(typeTable) / sizeof(typeTable[0]) == size, "GL error: DepthComparison and DepthComparisonGL don't match!");

		return typeTable[(unsigned)compareFunc];
	}

	IndexElementTypeGL translate(IndexElementType indexType)
	{
		static IndexElementTypeGL table[]
		{
			IndexElementTypeGL::BIT_16,
			IndexElementTypeGL::BIT_32,
		};

		static const unsigned size = (unsigned)IndexElementType::LAST - (unsigned)IndexElementType::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: IndexElementType and IndexElementTypeGL don't match!");

		return table[(unsigned)indexType];
	}

	PolygonSideGL translate(PolygonSide side)
	{
		static PolygonSideGL table[]
		{
			PolygonSideGL::BACK,
			PolygonSideGL::FRONT,
			PolygonSideGL::FRONT_BACK,
		};

		static const unsigned size = (unsigned)PolygonSide::LAST - (unsigned)PolygonSide::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: PolygonSide and PolygonSideGL don't match!");

		return table[(unsigned)side];
	}

	FillModeGL translate(FillMode type)
	{
		static FillModeGL table[]
		{
			FillModeGL::FILL,
			FillModeGL::LINE,
			FillModeGL::POINT,
		};

		static const unsigned size = (unsigned)FillMode::LAST - (unsigned)FillMode::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: PolygonRasterizationType and PolygonRasterizationTypeGL don't match!");

		return table[(unsigned)type];
	}

	WindingOrderGL translate(WindingOrder order)
	{
		static WindingOrderGL table[]
		{
			WindingOrderGL::CLOCKWISE,
			WindingOrderGL::COUNTER_CLOWCKWISE,
		};

		static const unsigned size = (unsigned)WindingOrder::LAST - (unsigned)WindingOrder::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: WindingOrder and WindingOrderGL don't match!");

		return table[(unsigned)order];
	}
}