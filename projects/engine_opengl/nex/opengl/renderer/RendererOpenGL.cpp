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
#include <nex/opengl/texture/Image.hpp>
#include <nex/opengl/post_processing/blur/GaussianBlurGL.hpp>
#include <nex/opengl/model/Vob.hpp>
#include <nex/opengl/drawing/ModelDrawerGL.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/opengl/post_processing/SSAO_GL.hpp>
#include <nex/opengl/post_processing/HBAO_GL.hpp>
#include <nex/opengl/shadowing/CascadedShadowGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/opengl/shader/SkyBoxShaderGL.hpp>

using namespace std;
using namespace nex;
using namespace glm;

nex::Logger GLOBAL_RENDERER_LOGGER("Global Renderer");

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

RendererOpenGL::RendererOpenGL() : m_logger("RendererOpenGL"), mViewport(),
  backgroundColor(0.0f, 0.0f, 0.0f), modelDrawer(this),
  msaaSamples(1), smaa(nullptr), defaultRenderTarget(0,0, 0)
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
	glViewport(0, 0, mViewport.width, mViewport.height);
	glScissor(0, 0, mViewport.width, mViewport.height);
	defaultRenderTarget = BaseRenderTargetGL(mViewport.width, mViewport.height, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do

	// stencil buffering is enabled when needed!
	//glEnable(GL_STENCIL_TEST); // Enable stencil buffering

	// we want counter clock wise winding order
	glFrontFace(GL_CCW);

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
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// we want counter clock wise winding order
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_STENCIL_TEST);
	cullFaces(CullingMode::Back);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(1.0f);
	glClearStencil(0);
	GLCall(glStencilMask(0xFF));

	TextureManagerGL::get()->init();

	//checkGLErrors(BOOST_CURRENT_FUNCTION);
}

void RendererOpenGL::newFrame()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
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

void RendererOpenGL::blitRenderTargets(BaseRenderTargetGL* src, BaseRenderTargetGL* dest, const Dimension& dim, int renderComponents)
{
	int componentsGL = getRenderComponentsGL(renderComponents);
	dest->copyFrom(src, dim, componentsGL);
}

void RendererOpenGL::clearRenderTarget(BaseRenderTargetGL * renderTarget, int renderComponents)
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->getFrameBuffer()));
	int renderComponentsComponentsGL = getRenderComponentsGL(renderComponents);
	GLCall(glClear(renderComponentsComponentsGL));
}

CubeDepthMapGL* RendererOpenGL::createCubeDepthMap(int width, int height)
{
	cubeDepthMaps.emplace_back(move(CubeDepthMapGL(width, height)));
	return &cubeDepthMaps.back();
}

CubeRenderTargetGL * RendererOpenGL::createCubeRenderTarget(int width, int height, const TextureData& data)
{
	CubeRenderTargetGL result(width, height, data);

	cubeRenderTargets.emplace_back(move(result));
	return &cubeRenderTargets.back();
}

GLint RendererOpenGL::getCurrentRenderTarget() const
{
	GLint drawFboId = 0;
	GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId));
	return drawFboId;
}

BaseRenderTargetGL * RendererOpenGL::getDefaultRenderTarget()
{
	return &defaultRenderTarget;
}

RenderTargetGL* RendererOpenGL::create2DRenderTarget(int width, int height, const TextureData& data, int samples) {

	return createRenderTargetGL(width, height, data, samples, GL_DEPTH_STENCIL);
}

void RendererOpenGL::clearFrameBuffer(GLuint frameBuffer, vec4 color, float depthValue, int stencilValue)
{
	// backup current bound drawing frame buffer
	GLint drawFboId = getCurrentRenderTarget();

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));
	//glClearColor(color.r, color.g, color.b, color.a);

	glViewport(mViewport.x, mViewport.y, mViewport.width, mViewport.height);
	glScissor(mViewport.x, mViewport.y, mViewport.width, mViewport.height);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClearDepth(depthValue);
	glClearStencil(stencilValue);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	// restore frame buffer
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, drawFboId));
}

DepthMapGL* RendererOpenGL::createDepthMap(int width, int height)
{
	depthMaps.emplace_back(move(DepthMapGL(width, height)));
	return &depthMaps.back();
}

RenderTargetGL* RendererOpenGL::createRenderTarget(int samples)
{
	TextureData data;
	data.generateMipMaps = false;
	data.minFilter = TextureFilter::Linear;
	data.magFilter = TextureFilter::Linear;
	data.colorspace = ColorSpace::RGB;
	data.internalFormat = InternFormat::RGB32F;
	data.pixelDataType = PixelDataType::FLOAT;
	data.uvTechnique = TextureUVTechnique::ClampToEdge;

	int ssaaSamples = 1;
	
	//return createRenderTargetGL(width * ssaaSamples, height * ssaaSamples, data, samples, GL_DEPTH_STENCIL); //GL_RGBA
	return createRenderTargetGL(mViewport.width * ssaaSamples, mViewport.height * ssaaSamples, data, samples, GL_DEPTH32F_STENCIL8); //GL_RGBA
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
	} else
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
	} else
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

int RendererOpenGL::getRenderComponentsGL(int renderComponents)
{
	int componentsGL = 0;
	if (renderComponents & RenderComponent::Color) componentsGL |= GL_COLOR_BUFFER_BIT;
	if (renderComponents & RenderComponent::Depth) componentsGL |= GL_DEPTH_BUFFER_BIT;
	if (renderComponents & RenderComponent::Stencil) componentsGL |= GL_STENCIL_BUFFER_BIT;

	return componentsGL;
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

void RendererOpenGL::readback(const TextureGL* texture, TextureTarget target, unsigned mipmapLevel, ColorSpace format,
	PixelDataType type, void* dest)
{
	const GLuint textureID = texture->getTexture();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLCall(glActiveTexture(GL_TEXTURE0));
	if (Texture::isCubeTarget(target))
	{
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));
	} else
	{
		GLCall(glBindTexture(static_cast<GLenum>(target), textureID));
	}

	GLCall(glGetTexImage(static_cast<GLenum>(target), mipmapLevel, static_cast<GLenum>(format), static_cast<GLenum>(type), dest));
}

void RendererOpenGL::resize(int width, int height)
{
	mViewport.width = width;
	mViewport.height = height;
	defaultRenderTarget = BaseRenderTargetGL(width, height, 0);
}

void RendererOpenGL::release()
{
	if (effectLibrary.get() != nullptr)
		effectLibrary->release();

	for (auto it = cubeRenderTargets.begin(); it != cubeRenderTargets.end(); ) {
		CubeRenderTargetGL& target = *it;
		target.release();
		it = cubeRenderTargets.erase(it);
	}

	for (auto it = renderTargets.begin(); it != renderTargets.end();) {
		RenderTargetGL* target = *it;
		target->release();
		delete target;
		it = renderTargets.erase(it);
	}

	for (auto it = depthMaps.begin(); it != depthMaps.end();) {
		DepthMapGL& target = *it;
		target.release();
		it = depthMaps.erase(it);
	}

	for (auto it = depthMaps.begin(); it != depthMaps.end();) {
		DepthMapGL& target = *it;
		target.release();
		it = depthMaps.erase(it);
	}
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
	} else
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

void RendererOpenGL::useCubeDepthMap(CubeDepthMapGL* cubeDepthMap)
{

	CubeMapGL* cubeMap = cubeDepthMap->getCubeMap();

	GLCall(glViewport(mViewport.x, mViewport.y, cubeMap->getWidth(), cubeMap->getHeight()));
	glScissor(mViewport.x, mViewport.y, cubeMap->getWidth(), cubeMap->getHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, cubeDepthMap->getFramebuffer());
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeMap->getCubeMap(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glClearDepth(1.0);
	GLCall(glClear(GL_DEPTH_BUFFER_BIT));
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

void RendererOpenGL::useCubeRenderTarget(CubeRenderTargetGL * target, CubeMapGL::Side side, unsigned int mipLevel)
{
	CubeMapGL* cubeMap = target->getCubeMap();

	GLuint AXIS_SIDE = side;

	int width = target->getWidth();
	int height = target->getHeight();
	GLuint cubeMapTexture = cubeMap->getCubeMap();

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, target->getFrameBuffer()));

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, AXIS_SIDE, cubeMapTexture, mipLevel);
	GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void RendererOpenGL::useBaseRenderTarget(BaseRenderTargetGL * target)
{

	int width = target->getWidth();
	int height = target->getHeight();
	GLCall(glViewport(0, 0, width, height));
	glScissor(0, 0, width, height);
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, target->getFrameBuffer()));

	// clear the stencil (with 1.0) and depth (with 0) buffer of the screen buffer 
	//glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.0f, 0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT);
	//glDisable(GL_FRAMEBUFFER_SRGB);
}

void RendererOpenGL::useScreenTarget()
{
	useBaseRenderTarget(&defaultRenderTarget);
}

void RendererOpenGL::useVarianceShadowMap(RenderTargetGL* source)
{
	GLCall(glViewport(0, 0, source->getWidth(), source->getHeight()));
	glScissor(0, 0, mViewport.width, mViewport.height);
	glBindFramebuffer(GL_FRAMEBUFFER, source->getFrameBuffer());
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

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

void RendererOpenGL::__clearRenderTarget(RenderTargetGL* renderTarget, bool releasedAllocatedMemory)
{
	if (releasedAllocatedMemory && renderTarget->frameBuffer != GL_FALSE)
	{
		GLCall(glDeleteFramebuffers(1, &renderTarget->frameBuffer));
		glDeleteTextures(1, &renderTarget->renderBuffer);
		GLCall(glDeleteRenderbuffers(1, &renderTarget->textureBuffer->textureID));
	}

	renderTarget->frameBuffer = GL_FALSE;
	renderTarget->renderBuffer = GL_FALSE;
	renderTarget->textureBuffer->setTexture(GL_FALSE);
}

CubeRenderTargetGL* RendererOpenGL::renderCubeMap(int width, int height, TextureGL* equirectangularMap)
{
	EquirectangularSkyBoxShaderGL* shader = dynamic_cast<EquirectangularSkyBoxShaderGL*>(getShaderManager()->getShader(ShaderType::SkyBoxEquirectangular));
	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);

	shader->bind();
	shader->setSkyTexture(equirectangularMap);
	shader->setProjection(projection);

	Vob skyBox ("misc/SkyBoxCube.obj", ShaderType::BlinnPhongTex);
	
	TextureData textureData = {
		TextureFilter::Linear, 
		TextureFilter::Linear, 
		TextureUVTechnique::ClampToEdge, 
		ColorSpace::RGB,  
		PixelDataType::FLOAT, 
		InternFormat::RGB32F, 
		false};

	CubeRenderTargetGL*  result = createCubeRenderTarget(width, height, std::move(textureData));

	//view matrices;
	const mat4 views[] = {
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_X), //right; sign of up vector is not important
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_X), //left
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Y), //top
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Y), //bottom
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::POSITIVE_Z), //back
		CubeMapGL::getViewLookAtMatrixRH(CubeMapGL::NEGATIVE_Z) //front
	};


	//set the viewport to the dimensoion of the cubemap
	GLCall(glViewport(0,0, width, height));
	glScissor(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, result->getFrameBuffer());

	for (int i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result->getCubeMapGL(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//render into the texture
		shader->setView(views[i]);
		modelDrawer.draw(skyBox.getModel(), shader);
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	//register and return the cubemap
	return result;
}

RenderTargetGL* RendererOpenGL::createRenderTargetGL(int width, int height, const TextureData& data,
	GLuint samples, GLuint depthStencilType)
{
	assert(samples >= 1);

	GLClearError();

	RenderTargetGL* result;

	if (samples > 1)
	{
		result = RenderTargetGL::createMultisampled(width, height, data, samples, depthStencilType);
	}
	else
	{
		result = RenderTargetGL::createSingleSampled(width, height, data, depthStencilType);
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	renderTargets.push_back(result);
	return result;
}

std::unique_ptr<SSAO_DeferredGL> RendererOpenGL::createDeferredSSAO()
{
	return std::make_unique<SSAO_DeferredGL>(mViewport.width, mViewport.height, &modelDrawer);
}

std::unique_ptr<hbao::HBAO_GL> RendererOpenGL::createHBAO()
{
	return std::make_unique<hbao::HBAO_GL>(mViewport.width, mViewport.height, &modelDrawer);
}

EffectLibraryGL* RendererOpenGL::getEffectLibrary()
{
	return effectLibrary.get();
}

RenderTargetGL* RendererOpenGL::createVarianceShadowMap(int width, int height)
{
	RenderTargetGL* target = RenderTargetGL::createVSM(width, height);
	renderTargets.push_back(target);
	return target;
}

void RendererOpenGL::cullFaces(CullingMode mode)
{
	if (mode == CullingMode::Front)
	{
		GLCall(glCullFace(GL_FRONT));

		// TODO this is needed for rendering shadow maps => put it on a more suitable place
		//glEnable(GL_POLYGON_OFFSET_FILL);
		//glPolygonOffset(-1.0f, -1.0f);
	} else
	{
		//glDisable(GL_POLYGON_OFFSET_FILL);
		GLCall(glCullFace(GL_BACK));
	}
}

void RendererOpenGL::destroyCubeRenderTarget(CubeRenderTargetGL * target)
{
	CubeRenderTargetGL* targetGL = dynamic_cast<CubeRenderTargetGL*>(target);
	assert(targetGL != nullptr);

	for (auto it = cubeRenderTargets.begin(); it != cubeRenderTargets.end(); ++it)
	{
		if (&(*it) == targetGL)
		{
			targetGL->release();
			cubeRenderTargets.erase(it);
			break;
		}
	}
}

void RendererOpenGL::destroyRenderTarget(RenderTargetGL* target)
{
	for (auto it = renderTargets.begin(); it != renderTargets.end(); ++it)
	{
		if ((*it) == target)
		{
			delete target;
			renderTargets.erase(it);
			break;
		}
	}
}