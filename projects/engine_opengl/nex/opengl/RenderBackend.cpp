#include <nex/RenderBackend.hpp>
#include <nex/opengl/RenderBackendGL.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/mesh/Vob.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <nex/opengl/opengl.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include "nex/texture/Attachment.hpp"
#include "nex/post_processing/PostProcessor.hpp"
#include "nex/shader/SkyBoxShader.hpp"
#include <nex/EffectLibrary.hpp>

using namespace std;
using namespace nex;
using namespace glm;

namespace nex
{

	class RenderBackend::Impl
	{
	public:

		Impl() : m_logger("RenderBackend - OPENGL"),
			backgroundColor(0.0f, 0.0f, 0.0f),
			msaaSamples(1), defaultRenderTarget(nullptr)
		{
			
		}

		~Impl() = default;

		//Note: we use public qualifier as this class is private and only used for the RenderBackend!
		glm::vec3 backgroundColor;
		std::unique_ptr<EffectLibrary> mEffectLibrary;
		unsigned int msaaSamples;
		std::unique_ptr<RenderTarget2D> defaultRenderTarget;
		//std::map<unsigned, RenderTargetBlendDesc> mBlendDescs;
		//BlendState mBlendState;

		nex::Logger m_logger{ "RenderBackend" };
		Blender mBlender;
		DepthBuffer mDepthBuffer;
		Rasterizer mRasterizer;
		StencilTest mStencilTest;
		Viewport mViewport;
	};


	Blender::Blender()
	{
		mImpl = make_unique<BlenderGL>();
	}

	void Blender::enableBlend(bool enable)
	{
		((BlenderGL*)mImpl.get())->enableBlend(enable);
	}

	void Blender::enableAlphaToCoverage(bool enable)
	{
		((BlenderGL*)mImpl.get())->enableAlphaToCoverage(enable);
	}

	void Blender::setSampleConverage(float sampleCoverage, bool invert)
	{
		((BlenderGL*)mImpl.get())->setSampleConverage(sampleCoverage, invert);
	}

	void Blender::setConstantBlendColor(const glm::vec4& color)
	{
		((BlenderGL*)mImpl.get())->setConstantBlendColor(color);
	}

	void Blender::setGlobalBlendDesc(const BlendDesc& desc)
	{
		((BlenderGL*)mImpl.get())->setGlobalBlendDesc(desc);
	}

	void Blender::setState(const BlendState& state)
	{
		((BlenderGL*)mImpl.get())->setState(state);
	}

	void Blender::setRenderTargetBlending(const RenderTargetBlendDesc & blendDesc)
	{
		((BlenderGL*)mImpl.get())->setRenderTargetBlending(blendDesc);
	}

	DepthBuffer::DepthBuffer()
	{
		mImpl = std::make_unique<DepthBufferGL>();
	}

	void DepthBuffer::enableDepthBufferWriting(bool enable)
	{
		((DepthBufferGL*)mImpl.get())->enableDepthBufferWriting(enable);
	}

	void DepthBuffer::enableDepthTest(bool enable)
	{
		((DepthBufferGL*)mImpl.get())->enableDepthTest(enable);
	}

	void DepthBuffer::enableDepthClamp(bool enable)
	{
		((DepthBufferGL*)mImpl.get())->enableDepthClamp(enable);
	}

	void DepthBuffer::setDefaultDepthFunc(CompareFunction depthFunc)
	{
		((DepthBufferGL*)mImpl.get())->setDefaultDepthFunc(depthFunc);
	}

	void DepthBuffer::setDepthRange(const Range& range)
	{
		((DepthBufferGL*)mImpl.get())->setDepthRange(range);
	}

	void DepthBuffer::setState(const State& state)
	{
		((DepthBufferGL*)mImpl.get())->setState(state);
	}

	Rasterizer::Rasterizer()
	{
		mImpl = make_unique<RasterizerGL>();
	}

	void nex::Rasterizer::setFillMode(FillMode fillMode, PolygonSide faceSide)
	{
		((RasterizerGL*)mImpl.get())->setFillMode(fillMode, faceSide);
	}

	void nex::Rasterizer::setCullMode(PolygonSide faceSide)
	{
		((RasterizerGL*)mImpl.get())->setCullMode(faceSide);
	}

	void nex::Rasterizer::setFrontCounterClockwise(bool set)
	{
		((RasterizerGL*)mImpl.get())->setFrontCounterClockwise(set);
	}

	void nex::Rasterizer::setDepthBias(float slopeScale, float unit, float clamp)
	{
		((RasterizerGL*)mImpl.get())->setDepthBias(slopeScale, unit, clamp);
	}

	void nex::Rasterizer::setState(const RasterizerState& state)
	{
		((RasterizerGL*)mImpl.get())->setState(state);
	}

	void nex::Rasterizer::enableFaceCulling(bool enable)
	{
		((RasterizerGL*)mImpl.get())->enableFaceCulling(enable);
	}

	void nex::Rasterizer::enableScissorTest(bool enable)
	{
		((RasterizerGL*)mImpl.get())->enableScissorTest(enable);
	}

	void nex::Rasterizer::enableMultisample(bool enable)
	{
		((RasterizerGL*)mImpl.get())->enableMultisample(enable);
	}

	void nex::Rasterizer::enableOffsetPolygonFill(bool enable)
	{
		((RasterizerGL*)mImpl.get())->enableOffsetPolygonFill(enable);
	}

	void nex::Rasterizer::enableOffsetLine(bool enable)
	{
		((RasterizerGL*)mImpl.get())->enableOffsetLine(enable);
	}

	void nex::Rasterizer::enableOffsetPoint(bool enable)
	{
		((RasterizerGL*)mImpl.get())->enableOffsetPoint(enable);
	}

	StencilTest::StencilTest()
	{
		mImpl = make_unique<StencilTestGL>();
	}

	void StencilTest::enableStencilTest(bool enable)
	{
		((StencilTestGL*)mImpl.get())->enableStencilTest(enable);
	}

	void StencilTest::setCompareFunc(CompareFunction func, int referenceValue, unsigned mask)
	{
		((StencilTestGL*)mImpl.get())->setCompareFunc(func, referenceValue, mask);
	}

	void StencilTest::setOperations(Operation stencilFail, Operation depthFail, Operation depthPass)
	{
		((StencilTestGL*)mImpl.get())->setOperations(stencilFail, depthFail, depthPass);
	}

	void StencilTest::setState(const State& state)
	{
		((StencilTestGL*)mImpl.get())->setState(state);
	}

	RenderBackend::RenderBackend() :
	mPimpl(std::make_unique<Impl>())
	{
		//__clearRenderTarget(&singleSampledScreenBuffer, false);
		//__clearRenderTarget(&multiSampledScreenBuffer, false);
	}

	RenderBackend::~RenderBackend()
	{
		//delete mPimpl;
		//mPimpl = nullptr;
	}

	void RenderBackend::init()
	{
		LOG(mPimpl->m_logger, Info) << "Initializing...";
		//checkGLErrors(BOOST_CURRENT_FUNCTION);

		getRasterizer()->enableScissorTest(true);
		setViewPort(0, 0, mPimpl->mViewport.width, mPimpl->mViewport.height);
		setScissor(0, 0, mPimpl->mViewport.width, mPimpl->mViewport.height);
		mPimpl->defaultRenderTarget = make_unique<RenderTarget2D>(make_unique<RenderTarget2DGL>(GL_FALSE, mPimpl->mViewport.width, mPimpl->mViewport.height));
		mPimpl->defaultRenderTarget->bind();
		mPimpl->defaultRenderTarget->clear(RenderComponent::Color);
		GLCall(glClearColor(1.0, 0.0, 0.0, 1.0)); // TODO abstract

		getDepthBuffer()->enableDepthTest(true);
		getDepthBuffer()->setDefaultDepthFunc(CompareFunction::LESS_EQUAL);

		getDepthBuffer()->enableDepthBufferWriting(true);

		// enable alpha blending
		//glEnable(GL_BLEND); // TODO
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//checkGLErrors(BOOST_CURRENT_FUNCTION);

		//checkGLErrors(BOOST_CURRENT_FUNCTION);

		mPimpl->mEffectLibrary = make_unique<EffectLibrary>(mPimpl->mViewport.width, mPimpl->mViewport.height);

		/*ImageLoaderGL imageLoader;
		GenericImageGL image = imageLoader.loadImageFromDisc("testImage.dds");
		if (image.pixels)
		{
			delete[] image.pixels;
			image.pixels = nullptr;
		}*/


		//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		GLCall(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS)); // TODO abstract?

		// we want counter clock wise winding order
		getRasterizer()->setFrontCounterClockwise(true);
		getStencilTest()->enableStencilTest(true);
		getRasterizer()->enableFaceCulling(true);
		getRasterizer()->setCullMode(PolygonSide::BACK);

		//GLCall(glClearColor(0.0, 0.0, 0.0, 1.0));
		GLCall(glClearDepth(1.0f));
		GLCall(glClearStencil(0)); // TODO abstract?
		GLCall(glStencilMask(0xFF)); // TODO abstract ?
		getStencilTest()->setCompareFunc(CompareFunction::LESS_EQUAL, 0, 0xFF); // TODO: Is it right?

		TextureManager::get()->init();

		//checkGLErrors(BOOST_CURRENT_FUNCTION);
	}

	void RenderBackend::newFrame()
	{
		getRasterizer()->setFillMode(FillMode::FILL, PolygonSide::FRONT_BACK);
		getRasterizer()->enableFaceCulling(true);
		getRasterizer()->setCullMode(PolygonSide::BACK);
	}

	std::unique_ptr<CubeDepthMap> RenderBackend::createCubeDepthMap(int width, int height)
	{
		return std::unique_ptr<CubeDepthMap>(CubeDepthMap::create(width, height));
	}

	std::unique_ptr<CubeRenderTarget> nex::RenderBackend::createCubeRenderTarget(int width, int height, const TextureData& data)
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

	std::unique_ptr <RenderTarget2D> nex::RenderBackend::create2DRenderTarget(int width, int height, const TextureData& data, const TextureData& depthData, int samples) {
		RenderAttachment depth;
		depth.type = RenderAttachment::translate(depthData.internalFormat);
		depth.texture = make_shared<Texture2D>(width, height, depthData, nullptr);

		auto result = createRenderTargetGL(width, height, data, samples);
		result->useDepthAttachment(std::move(depth));
		return result;
	}

	std::unique_ptr <RenderTarget2D> nex::RenderBackend::createRenderTarget(int samples)
	{
		const int ssaaSamples = 1;

		const unsigned width = mPimpl->mViewport.width * ssaaSamples;
		const unsigned height = mPimpl->mViewport.height * ssaaSamples;

		TextureData depthData = TextureData::createDepth(CompareFunction::LESS_EQUAL,
			ColorSpace::DEPTH_STENCIL,
			PixelDataType::UNSIGNED_INT_24_8,
			InternFormat::DEPTH24_STENCIL8);

		return create2DRenderTarget(width, height, TextureData::createRenderTargetRGBAHDR(), depthData, samples);
	}

	void RenderBackend::drawWithIndices(Topology topology, unsigned indexCount, IndexElementType indexType)
	{
		GLCall(glDrawElements(translate(topology), indexCount, translate(indexType), nullptr));
	}

	void RenderBackend::endScene()
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		//glDisable(GL_POLYGON_OFFSET_FILL);
		//checkGLErrors(BOOST_CURRENT_FUNCTION);
	}

	RenderBackend* RenderBackend::get()
	{
		static RenderBackend backend;
		return &backend;
	}

	Blender * RenderBackend::getBlender()
	{
		return &mPimpl->mBlender;
	}

	Rasterizer * RenderBackend::getRasterizer()
	{
		return &mPimpl->mRasterizer;
	}

	StencilTest* RenderBackend::getStencilTest()
	{
		return &mPimpl->mStencilTest;
	}

	RendererType RenderBackend::getType() const
	{
		return OPENGL;
	}

	const Viewport& RenderBackend::getViewport() const
	{
		return mPimpl->mViewport;
	}

	void RenderBackend::present()
	{
	}

	void RenderBackend::resize(int width, int height)
	{
		mPimpl->mViewport.width = width;
		mPimpl->mViewport.height = height;
		mPimpl->defaultRenderTarget = make_unique<RenderTarget2D>(make_unique<RenderTarget2DGL>(GL_FALSE, mPimpl->mViewport.width, mPimpl->mViewport.height));
		mPimpl->mEffectLibrary->resize(width, height);
	}

	void RenderBackend::release()
	{
		mPimpl->mEffectLibrary.reset(nullptr);
	}

	void RenderBackend::setBackgroundColor(const glm::vec3& color)
	{
		mPimpl->backgroundColor = color;
	}

	void RenderBackend::setLineThickness(float thickness)
	{
		assert(thickness >= 0.0f);
		GLCall(glLineWidth(thickness));
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
		//LOG(logClient, Debug) << "set view port called: " << width << ", " << height;

		//if (effectLibrary)
		//	effectLibrary->getGaussianBlur()->init();
	}

	/*void RendererOpenGL::useDepthMap(DepthMap* depthMap)
	{
		DepthMapGL* map = dynamic_cast<DepthMapGL*>(depthMap);
		assert(map != nullptr);
		TextureGL* textureGL = static_cast<TextureGL*>(map->getTexture());

		glViewport(xPos, yPos, map->getWidth(), map->getHeight());
		glScissor(xPos, yPos, map->getWidth(), map->getHeight());
		glBindFramebuffer(GL_FRAMEBUFFER, map->getFramebuffer());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureGL->getTexture(), 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		//glEnable(GL_POLYGON_OFFSET_FILL);
		//glPolygonOffset(1.0f, 15000.0f);

		//glClear(GL_DEPTH_BUFFER_BIT);
	}*/

	CubeRenderTarget* RenderBackend::renderCubeMap(int width, int height, Texture* equirectangularMap)
	{
		auto* shader = mPimpl->mEffectLibrary->getEquirectangularSkyBoxShader();
		const mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

		shader->bind();
		shader->setSkyTexture(equirectangularMap);
		shader->setProjection(projection);

		Vob skyBox("misc/SkyBoxCube.obj", MaterialType::BlinnPhong);

		TextureData textureData = {
			TextureFilter::Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			InternFormat::RGB32F,
			false };

		auto target = createCubeRenderTarget(width, height, std::move(textureData));

		//view matrices;
		const mat4 views[] = {
			CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_X), //right; sign of up vector is not important
			CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_X), //left
			CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Y), //top
			CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Y), //bottom
			CubeMap::getViewLookAtMatrixRH(CubeMapSide::POSITIVE_Z), //back
			CubeMap::getViewLookAtMatrixRH(CubeMapSide::NEGATIVE_Z) //front
		};


		//set the viewport to the dimensoion of the cubemap
		//GLCall(glViewport(0, 0, width, height));
		//GLCall(glScissor(0, 0, width, height));
		setViewPort(0, 0, width, height);
		setScissor(0, 0, width, height);
		target->bind();

		CubeMapGL* cubeMap = (CubeMapGL*)target->getColorAttachments()[0].texture->getImpl();
		

		for (int i = 0; i < 6; ++i) {
			//GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, *cubeMap->getTexture(), 0));
			target->useSide((CubeMapSide) i, 0);
			//GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
			target->clear(RenderComponent::Color | RenderComponent::Depth);

			//render into the texture
			shader->setView(views[i]);

			StaticMeshDrawer::draw(skyBox.getModel(), shader);
		}

		//GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		//register and return the cubemap
		return target.release();
	}

	std::unique_ptr <RenderTarget2D> RenderBackend::createRenderTargetGL(int width, int height, const TextureData& data,
		unsigned samples)
	{
		assert(samples >= 1);

		//GLClearError();

		return make_unique<RenderTarget2D>(width, height, data, samples);
	}

	EffectLibrary* RenderBackend::getEffectLibrary()
	{
		return mPimpl->mEffectLibrary.get();
	}

	/*RenderTarget* RendererOpenGL::createVarianceShadowMap(int width, int height)
	{
		RenderTarget* target = RenderTarget::createVSM(width, height);
		renderTargets.push_back(target);
		return target;
	}*/

	void RenderBackend::drawArray(Topology primitiveType, unsigned startingIndex,
		unsigned indexCount)
	{
		const GLenum primitiveTypeGL = translate(primitiveType);
		GLCall(glDrawArrays(primitiveTypeGL, startingIndex, indexCount));
	}
}