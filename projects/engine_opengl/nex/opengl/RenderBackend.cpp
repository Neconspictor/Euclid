#include <nex/RenderBackend.hpp>
#include <nex/opengl/RenderBackendGL.hpp>
#include <nex/shader/ShaderManager.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/mesh/Vob.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include "nex/opengl/texture/RenderTargetGL.hpp"
#include <nex/opengl/opengl.hpp>
#include "nex/drawing/StaticMeshDrawer.hpp"

using namespace std;
using namespace nex;
using namespace glm;

namespace nex
{

	GLuint translate(Topology topology)
	{
		static TopologyGL table[]
		{
			LINES,
			LINES_ADJACENCY,
			LINE_LOOP,
			LINE_STRIP,
			LINE_STRIP_ADJACENCY,
			PATCHES,
			POINTS,
			TRIANGLES,
			TRIANGLES_ADJACENCY,
			TRIANGLE_FAN,
			TRIANGLE_STRIP,
			TRIANGLE_STRIP_ADJACENCY,
		};

		static const unsigned size = (unsigned)Topology::LAST - (unsigned)Topology::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: Topology and TopologyGL don't match!");

		return table[(unsigned)topology];
	}
	GLuint translate(IndexElementType indexType)
	{
		static IndexElementTypeGL table[]
		{
			BIT_16,
			BIT_32,
		};

		static const unsigned size = (unsigned)IndexElementType::LAST - (unsigned)IndexElementType::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: IndexElementType and IndexElementTypeGL don't match!");

		return table[(unsigned)indexType];	
	}


	/*EffectLibrary::EffectLibrary(RenderBackend * renderer) : renderer(renderer)
	{
		gaussianBlur = make_unique<GaussianBlurGL>(renderer);
		gaussianBlur->init();
	}

	GaussianBlurGL* EffectLibrary::getGaussianBlur()
	{
		return gaussianBlur.get();
	}

	void EffectLibrary::release()
	{
		if (gaussianBlur.get() != nullptr)
			gaussianBlur->release();
	}*/




	RenderBackend::RenderBackend() : m_logger("RenderBackend - OPENGL"),
		backgroundColor(0.0f, 0.0f, 0.0f),
		msaaSamples(1), defaultRenderTarget(nullptr)
	{
		//__clearRenderTarget(&singleSampledScreenBuffer, false);
		//__clearRenderTarget(&multiSampledScreenBuffer, false);
	}

	RenderBackend::~RenderBackend()
	{
		release();
	}

	// An array of 3 vectors which represents 3 vertices
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};
	GLuint vertexbuffer;
	GLuint vertexArrayObjID;

	void RenderBackend::init()
	{
		LOG(m_logger, Info) << "Initializing...";
		//checkGLErrors(BOOST_CURRENT_FUNCTION);

		GLCall(glEnable(GL_SCISSOR_TEST));
		GLCall(glViewport(0, 0, mViewport.width, mViewport.height));
		GLCall(glScissor(0, 0, mViewport.width, mViewport.height));
		defaultRenderTarget = make_unique<RenderTarget2D>(make_unique<RenderTarget2DGL>(mViewport.width, mViewport.height, GL_FALSE, nullptr));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));
		GLCall(glClearColor(0.0, 0.0, 0.0, 1.0));

		GLCall(glEnable(GL_DEPTH_TEST)); // Enables Depth Testing
		GLCall(glDepthFunc(GL_LESS)); // The Type Of Depth Testing To Do

		// stencil buffering is enabled when needed!
		//glEnable(GL_STENCIL_TEST); // Enable stencil buffering

		// we want counter clock wise winding order
		GLCall(glFrontFace(GL_CCW));

		// only draw front faces
		enableBackfaceDrawing(false);

		enableDepthWriting(true);

		// enable alpha blending
		//glEnable(GL_BLEND); // TODO
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//checkGLErrors(BOOST_CURRENT_FUNCTION);

		//checkGLErrors(BOOST_CURRENT_FUNCTION);

		effectLibrary = make_unique<EffectLibrary>(this);

		/*ImageLoaderGL imageLoader;
		GenericImageGL image = imageLoader.loadImageFromDisc("testImage.dds");
		if (image.pixels)
		{
			delete[] image.pixels;
			image.pixels = nullptr;
		}*/


		//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		GLCall(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));

		// we want counter clock wise winding order
		GLCall(glEnable(GL_DEPTH_TEST)); // Enables Depth Testing
		GLCall(glDepthFunc(GL_LESS)); // The Type Of Depth Testing To Do
		GLCall(glFrontFace(GL_CCW));
		GLCall(glEnable(GL_CULL_FACE));
		GLCall(glEnable(GL_STENCIL_TEST));
		cullFaces(CullingMode::Back);

		GLCall(glClearColor(0.0, 0.0, 0.0, 1.0));
		GLCall(glClearDepth(1.0f));
		GLCall(glClearStencil(0));
		GLCall(glStencilMask(0xFF));

		TextureManager::get()->init();

		//checkGLErrors(BOOST_CURRENT_FUNCTION);
	}

	void RenderBackend::newFrame()
	{
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		GLCall(glEnable(GL_CULL_FACE));
		GLCall(glCullFace(GL_BACK));
	}

	void RenderBackend::beginScene()
	{
		// enable alpha blending
		//glEnable(GL_BLEND); // TODO
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		//glViewport(xPos, yPos, width, height);
		//glScissor(xPos, yPos, width, height);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

		//clearFrameBuffer(getCurrentRenderTarget(), { 0.5, 0.5, 0.5, 1 }, 1.0f, 0);

		//glStencilMask(0x00);

		//checkGLErrors(BOOST_CURRENT_FUNCTION);
	}

	CubeDepthMap* RenderBackend::createCubeDepthMap(int width, int height)
	{
		Guard<CubeDepthMap> guard;
		guard = CubeDepthMap::create(width, height);
		cubeDepthMaps.push_back(guard.reset());
		return cubeDepthMaps.back();
	}

	CubeRenderTarget * nex::RenderBackend::createCubeRenderTarget(int width, int height, const TextureData& data)
	{
		cubeRenderTargets.emplace_back(make_unique<CubeRenderTarget>(width, height, data));
		return cubeRenderTargets.back().get();
	}

	RenderTarget2D* nex::RenderBackend::getDefaultRenderTarget()
	{
		return defaultRenderTarget.get();
	}

	RenderTarget2D* nex::RenderBackend::create2DRenderTarget(int width, int height, const TextureData& data, int samples) {

		DepthStencilDesc desc;
		auto depthTexture = make_shared<DepthStencilMap>(width, height, desc);
		auto result = createRenderTargetGL(width, height, data, samples, depthTexture);
		return result;
	}

	RenderTarget2D* nex::RenderBackend::createRenderTarget(int samples)
	{
		TextureData data;
		data.generateMipMaps = false;
		data.minFilter = TextureFilter::Linear;
		data.magFilter = TextureFilter::Linear;
		data.colorspace = ColorSpace::RGB;
		data.internalFormat = InternFormat::RGB32F;
		data.pixelDataType = PixelDataType::FLOAT;
		data.wrapR = TextureUVTechnique::ClampToEdge;
		data.wrapS = TextureUVTechnique::ClampToEdge;
		data.wrapT = TextureUVTechnique::ClampToEdge;

		const int ssaaSamples = 1;

		const unsigned width = mViewport.width * ssaaSamples;
		const unsigned height = mViewport.height * ssaaSamples;

		DepthStencilDesc desc;
		auto depthTexture = make_shared<DepthStencilMap>(width, height, desc);
		auto result = createRenderTargetGL(width, height, data, samples, depthTexture); //GL_RGBA

		return result;
	}

	void RenderBackend::enableAlphaBlending(bool enable)
	{
		if (!enable) {
			GLCall(glDisable(GL_BLEND));
			return;
		}

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	void RenderBackend::enableBackfaceDrawing(bool enable)
	{
		if (enable)
		{
			GLCall(glDisable(GL_CULL_FACE));
		}
		else
		{
			GLCall(glEnable(GL_CULL_FACE));
			//glCullFace(GL_BACK);
		}
	}

	void RenderBackend::enableDepthWriting(bool enable)
	{
		const GLuint value = enable ? GL_TRUE : GL_FALSE;
		GLCall(glDepthMask(value));
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

	RendererType RenderBackend::getType() const
	{
		return OPENGL;
	}

	const Viewport& RenderBackend::getViewport() const
	{
		return mViewport;
	}

	void RenderBackend::present()
	{
	}

	void RenderBackend::resize(int width, int height)
	{
		mViewport.width = width;
		mViewport.height = height;
		defaultRenderTarget = make_unique<RenderTarget2D>(make_unique<RenderTarget2DGL>(mViewport.width, mViewport.height, GL_FALSE, nullptr));
	}

	void RenderBackend::release()
	{
		if (effectLibrary.get() != nullptr)
			effectLibrary->release();
	}

	void RenderBackend::setBackgroundColor(const glm::vec3& color)
	{
		backgroundColor = color;
	}

	void RenderBackend::setMSAASamples(unsigned int samples)
	{
		if (samples == 0)
		{
			// Samples smaller one cannot be handled by OpenGL
			msaaSamples = 1;
		}
		else
		{
			msaaSamples = samples;
		}
	}

	void RenderBackend::setViewPort(int x, int y, int width, int height)
	{
		mViewport.x = x;
		mViewport.y = y;
		mViewport.width = width;
		mViewport.height = height;

		GLCall(glViewport(x, y, width, height));
		GLCall(glScissor(x, y, width, height));
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
		auto* shaderManager = ShaderManager::get();
		EquirectangularSkyBoxShader* shader = dynamic_cast<EquirectangularSkyBoxShader*>(shaderManager->getShader(ShaderType::SkyBoxEquirectangular));
		mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

		shader->bind();
		shader->setSkyTexture(equirectangularMap);
		shader->setProjection(projection);

		Vob skyBox("misc/SkyBoxCube.obj", ShaderType::BlinnPhongTex);

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

		Guard<CubeRenderTarget>  target;
		target = createCubeRenderTarget(width, height, std::move(textureData));

		//view matrices;
		const mat4 views[] = {
			CubeMap::getViewLookAtMatrixRH(CubeMap::Side::POSITIVE_X), //right; sign of up vector is not important
			CubeMap::getViewLookAtMatrixRH(CubeMap::Side::NEGATIVE_X), //left
			CubeMap::getViewLookAtMatrixRH(CubeMap::Side::POSITIVE_Y), //top
			CubeMap::getViewLookAtMatrixRH(CubeMap::Side::NEGATIVE_Y), //bottom
			CubeMap::getViewLookAtMatrixRH(CubeMap::Side::POSITIVE_Z), //back
			CubeMap::getViewLookAtMatrixRH(CubeMap::Side::NEGATIVE_Z) //front
		};


		//set the viewport to the dimensoion of the cubemap
		GLCall(glViewport(0, 0, width, height));
		GLCall(glScissor(0, 0, width, height));
		target->bind();

		CubeMapGL* cubeMap = (CubeMapGL*)target->getRenderResult()->getImpl();

		for (int i = 0; i < 6; ++i) {
			GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, *cubeMap->getTexture(), 0));
			GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

			//render into the texture
			shader->setView(views[i]);
			static StaticMeshDrawer drawer;

			drawer.draw(skyBox.getModel(), shader);
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		//register and return the cubemap
		return target.reset();
	}

	RenderTarget2D* RenderBackend::createRenderTargetGL(int width, int height, const TextureData& data,
		unsigned samples, std::shared_ptr<Texture> depthStencilMap)
	{
		assert(samples >= 1);

		GLClearError();

		auto result = make_unique<RenderTarget2D>(width, height, data, samples, std::move(depthStencilMap));

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		checkGLErrors(BOOST_CURRENT_FUNCTION);

		mRenderTargets.emplace_back(std::move(result));
		return mRenderTargets.back().get();
	}

	EffectLibrary* RenderBackend::getEffectLibrary()
	{
		return effectLibrary.get();
	}

	/*RenderTarget* RendererOpenGL::createVarianceShadowMap(int width, int height)
	{
		RenderTarget* target = RenderTarget::createVSM(width, height);
		renderTargets.push_back(target);
		return target;
	}*/

	void RenderBackend::cullFaces(CullingMode mode)
	{
		if (mode == CullingMode::Front)
		{
			GLCall(glCullFace(GL_FRONT));

			// TODO this is needed for rendering shadow maps => put it on a more suitable place
			//glEnable(GL_POLYGON_OFFSET_FILL);
			//glPolygonOffset(-1.0f, -1.0f);
		}
		else
		{
			//glDisable(GL_POLYGON_OFFSET_FILL);
			GLCall(glCullFace(GL_BACK));
		}
	}

	void RenderBackend::destroyCubeRenderTarget(CubeRenderTarget * target)
	{
		using type = std::unique_ptr<CubeRenderTarget>;
		cubeRenderTargets.remove_if([&](const type& it)->bool
		{
			return it.get() == target;
		});
	}

	void RenderBackend::destroyRenderTarget(RenderTarget2D* target)
	{
		using type = std::unique_ptr<RenderTarget2D>;
		mRenderTargets.remove_if([&](const type& it)->bool
		{
			return it.get() == target;
		});
	}
}