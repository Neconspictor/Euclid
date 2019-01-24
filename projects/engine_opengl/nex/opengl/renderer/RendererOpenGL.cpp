#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/exception/OpenglException.hpp>
#include <nex/opengl/model/ModelManagerGL.hpp>
#include <nex/opengl/drawing/ModelDrawerGL.hpp>
#include <nex/opengl/shader/ScreenShaderGL.hpp>
#include <glad/glad.h>
#include <GL/gl.h>
#include <nex/opengl/antialiasing/SMAA_GL.hpp>
#include <nex/opengl/post_processing/blur/GaussianBlurGL.hpp>
#include <nex/opengl/model/Vob.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/opengl/post_processing/SSAO_GL.hpp>
#include <nex/opengl/post_processing/HBAO_GL.hpp>
#include <nex/opengl/shadowing/CascadedShadowGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include "nex/opengl/texture/RenderTargetGL.hpp"

using namespace std;
using namespace nex;
using namespace glm;

nex::Logger GLOBAL_RENDERER_LOGGER("Global Renderer");

namespace nex
{

	void GLClearError()
	{

		unsigned int finite = 4096;
		GLuint errorCode = glGetError();

		while (errorCode && finite)
		{
			if (errorCode == GL_INVALID_OPERATION)
				--finite;
			errorCode = glGetError();
		};

		if (!finite)
		{
			static nex::Logger logger("[GLClearError]");
			logger.log(nex::LogLevel::Warning) << "Detected to many GL_INVALID_OPERATION errors. Assuming that no valid OpenGL context exists.";
			//LOG(logger) << "Detected to many GL_INVALID_OPERATION errors. Assuming that no valid OpenGL context exists.";
			throw_with_trace(std::runtime_error("Detected to many GL_INVALID_OPERATION errors"));
		}
	}

	bool GLLogCall()
	{
		static nex::Logger logger("OpenGL Error");

		bool noErrorsOccurred = true;

		while (GLenum error = glGetError())
		{
			logger.log(nex::LogLevel::Warning) << "Error occurred: " << GLErrorToString(error) << " (0x" << std::hex << error << ")";
			noErrorsOccurred = false;
		}

		return noErrorsOccurred;
	}

	std::string GLErrorToString(GLuint errorCode)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		default:							   error = "Unknown error code: " + std::to_string(errorCode);
		}

		return error;
	}


	EffectLibraryGL::EffectLibraryGL(RendererOpenGL * renderer) : renderer(renderer)
	{
		gaussianBlur = make_unique<GaussianBlurGL>(renderer);
		gaussianBlur->init();
	}

	GaussianBlurGL* EffectLibraryGL::getGaussianBlur()
	{
		return gaussianBlur.get();
	}

	void EffectLibraryGL::release()
	{
		if (gaussianBlur.get() != nullptr)
			gaussianBlur->release();
	}

	RendererOpenGL::RendererOpenGL() : m_logger("RendererOpenGL"),
		backgroundColor(0.0f, 0.0f, 0.0f), modelDrawer(this),
		msaaSamples(1), smaa(nullptr), defaultRenderTarget(nullptr)
	{
		//__clearRenderTarget(&singleSampledScreenBuffer, false);
		//__clearRenderTarget(&multiSampledScreenBuffer, false);

		shadingModelFactory = make_unique<ShadingModelFactoryGL>();
	}

	std::unique_ptr<CascadedShadowGL> RendererOpenGL::createCascadedShadow(unsigned int width, unsigned int height)
	{
		return make_unique<CascadedShadowGL>(width, height);
	}

	RendererOpenGL::~RendererOpenGL()
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

	void RendererOpenGL::init()
	{
		LOG(m_logger, Info) << "Initializing...";
		//checkGLErrors(BOOST_CURRENT_FUNCTION);

		GLCall(glEnable(GL_SCISSOR_TEST));
		GLCall(glViewport(0, 0, mViewport.width, mViewport.height));
		GLCall(glScissor(0, 0, mViewport.width, mViewport.height));
		defaultRenderTarget = make_unique<RenderTarget2D>(make_unique<RenderTarget2DGL>(mViewport.width, mViewport.height));
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


		smaa = make_unique<SMAA_GL>(this);
		smaa->init();

		effectLibrary = make_unique<EffectLibraryGL>(this);

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

		TextureManagerGL::get()->init();

		//checkGLErrors(BOOST_CURRENT_FUNCTION);
	}

	void RendererOpenGL::newFrame()
	{
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		GLCall(glEnable(GL_CULL_FACE));
		GLCall(glCullFace(GL_BACK));
	}

	void RendererOpenGL::beginScene()
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

	CubeDepthMap* RendererOpenGL::createCubeDepthMap(int width, int height)
	{
		Guard<CubeDepthMap> guard;
		guard = CubeDepthMap::create(width, height);
		cubeDepthMaps.push_back(guard.reset());
		return cubeDepthMaps.back();
	}

	CubeRenderTarget * nex::RendererOpenGL::createCubeRenderTarget(int width, int height, const TextureData& data)
	{
		cubeRenderTargets.emplace_back(make_unique<CubeRenderTarget>(width, height, data));
		return cubeRenderTargets.back().get();
	}

	GLint nex::RendererOpenGL::getCurrentRenderTarget() const
	{
		GLint drawFboId = 0;
		GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId));
		return drawFboId;
	}

	RenderTarget2D* nex::RendererOpenGL::getDefaultRenderTarget()
	{
		return defaultRenderTarget.get();
	}

	RenderTarget2D* nex::RendererOpenGL::create2DRenderTarget(int width, int height, const TextureData& data, int samples) {

		DepthStencilDesc desc;
		auto depthTexture = make_shared<DepthStencilMap>(width, height, desc);
		auto result = createRenderTargetGL(width, height, data, samples, depthTexture);
		return result;
	}

	RenderTarget2D* nex::RendererOpenGL::createRenderTarget(int samples)
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

	void RendererOpenGL::enableAlphaBlending(bool enable)
	{
		if (!enable) {
			GLCall(glDisable(GL_BLEND));
			return;
		}

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	void RendererOpenGL::enableBackfaceDrawing(bool enable)
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

	void RendererOpenGL::enableDepthWriting(bool enable)
	{
		if (enable)
		{
			GLCall(glDepthMask(GL_TRUE));
		}
		else
		{
			GLCall(glDepthMask(GL_FALSE));
		}
	}

	void RendererOpenGL::endScene()
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		//glDisable(GL_POLYGON_OFFSET_FILL);
		//checkGLErrors(BOOST_CURRENT_FUNCTION);
	}

	ModelDrawerGL* RendererOpenGL::getModelDrawer()
	{
		return &modelDrawer;
	}

	ModelManagerGL* RendererOpenGL::getModelManager()
	{
		return ModelManagerGL::get();
	}

	ShaderManagerGL* RendererOpenGL::getShaderManager()
	{
		return ShaderManagerGL::get();
	}

	ShadingModelFactoryGL& RendererOpenGL::getShadingModelFactory()
	{
		return *shadingModelFactory.get();
	}

	SMAA_GL* RendererOpenGL::getSMAA()
	{
		return smaa.get();
	}

	TextureManagerGL* RendererOpenGL::getTextureManager()
	{
		return TextureManagerGL::get();
	}

	RendererType RendererOpenGL::getType() const
	{
		return OPENGL;
	}

	const Viewport& RendererOpenGL::getViewport() const
	{
		return mViewport;
	}

	void RendererOpenGL::present()
	{
	}

	void RendererOpenGL::resize(int width, int height)
	{
		mViewport.width = width;
		mViewport.height = height;
		defaultRenderTarget = make_unique<RenderTarget2D>(make_unique<RenderTarget2DGL>(mViewport.width, mViewport.height));
	}

	void RendererOpenGL::release()
	{
		if (effectLibrary.get() != nullptr)
			effectLibrary->release();
	}

	void RendererOpenGL::setBackgroundColor(glm::vec3 color)
	{
		backgroundColor = move(color);
	}

	void RendererOpenGL::setMSAASamples(unsigned int samples)
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

	void RendererOpenGL::setViewPort(int x, int y, int width, int height)
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

	void RendererOpenGL::checkGLErrors(const string& errorPrefix)
	{
		return;
		// check if any gl related errors occured
		GLint errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			string error;
			switch (errorCode)
			{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			default:							   error = "Unknown error code: " + to_string(errorCode);
			}

			stringstream ss; ss << errorPrefix << ": Error occured: " << error;
			throw_with_trace(OpenglException(ss.str()));
		}
	}

	bool RendererOpenGL::checkGLErrorSilently()
	{
		// check if any gl related errors occured
		GLint errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			string error;
			switch (errorCode)
			{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			default:							   error = "Unknown error code: " + to_string(errorCode);
			}

			return true;
		}

		return false;
	}

	CubeRenderTarget* RendererOpenGL::renderCubeMap(int width, int height, Texture* equirectangularMap)
	{
		EquirectangularSkyBoxShader* shader = dynamic_cast<EquirectangularSkyBoxShader*>(getShaderManager()->getShader(ShaderType::SkyBoxEquirectangular));
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
			modelDrawer.draw(skyBox.getModel(), shader);
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		//register and return the cubemap
		return target.reset();
	}

	RenderTarget2D* RendererOpenGL::createRenderTargetGL(int width, int height, const TextureData& data,
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

	std::unique_ptr<SSAO_DeferredGL> RendererOpenGL::createDeferredSSAO()
	{
		return std::make_unique<SSAO_DeferredGL>(mViewport.width, mViewport.height, &modelDrawer);
	}

	std::unique_ptr<HBAO_GL> RendererOpenGL::createHBAO()
	{
		return std::make_unique<HBAO_GL>(mViewport.width, mViewport.height, &modelDrawer);
	}

	EffectLibraryGL* RendererOpenGL::getEffectLibrary()
	{
		return effectLibrary.get();
	}

	/*RenderTarget* RendererOpenGL::createVarianceShadowMap(int width, int height)
	{
		RenderTarget* target = RenderTarget::createVSM(width, height);
		renderTargets.push_back(target);
		return target;
	}*/

	void RendererOpenGL::cullFaces(CullingMode mode)
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

	void RendererOpenGL::destroyCubeRenderTarget(CubeRenderTarget * target)
	{
		using type = std::unique_ptr<CubeRenderTarget>;
		cubeRenderTargets.remove_if([&](const type& it)->bool
		{
			return it.get() == target;
		});
	}

	void RendererOpenGL::destroyRenderTarget(RenderTarget2D* target)
	{
		using type = std::unique_ptr<RenderTarget2D>;
		mRenderTargets.remove_if([&](const type& it)->bool
		{
			return it.get() == target;
		});
	}
}