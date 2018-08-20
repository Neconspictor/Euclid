#include <renderer/RendererOpenGL.hpp>
#include <shader/ShaderManagerGL.hpp>
#include <texture/TextureManagerGL.hpp>
#include <platform/exception/OpenglException.hpp>
#include <model/ModelManagerGL.hpp>
#include <drawing/ModelDrawerGL.hpp>
#include <shader/ScreenShaderGL.hpp>
#include <glad/glad.h>
#include <GL/gl.h>
#include <antialiasing/SMAA_GL.hpp>
#include <fstream>
#include <texture/ImageLoaderGL.hpp>
#include<post_processing/blur/GaussianBlurGL.hpp>
#include <shader/SkyBoxShader.hpp>
#include <model/Vob.hpp>
#include <drawing/ModelDrawer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <post_processing/SSAO_GL.hpp>
#include <post_processing/HBAO_GL.hpp>

using namespace std;
using namespace platform;
using namespace glm;


EffectLibraryGL::EffectLibraryGL(RendererOpenGL * renderer) : EffectLibrary(), renderer(renderer)
{
	gaussianBlur = make_unique<GaussianBlurGL>(renderer);
	gaussianBlur->init();
}

GaussianBlur* EffectLibraryGL::getGaussianBlur()
{
	return gaussianBlur.get();
}

void EffectLibraryGL::release()
{
	if (gaussianBlur.get() != nullptr)
		gaussianBlur->release();
}

RendererOpenGL::RendererOpenGL() : RenderBackend(), 
  backgroundColor(0.0f, 0.0f, 0.0f), modelDrawer(this),
  msaaSamples(1), smaa(nullptr), defaultRenderTarget(0,0, 0)
{	
	logClient.setPrefix("[RendererOpenGL]");

	//clearRenderTarget(&singleSampledScreenBuffer, false);
	//clearRenderTarget(&multiSampledScreenBuffer, false);

	smaa = make_unique<SMAA_GL>(this);
	shadingModelFactory = make_unique<ShadingModelFactoryGL>(this);
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
	LOG(logClient, Info) << "Initializing...";
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	glEnable(GL_SCISSOR_TEST);
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
	defaultRenderTarget = BaseRenderTargetGL(width, height, 0);
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

	checkGLErrors(BOOST_CURRENT_FUNCTION);

	ShaderGL::initShaderFileSystem();

	checkGLErrors(BOOST_CURRENT_FUNCTION);

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
	glStencilMask(0xFF);

	checkGLErrors(BOOST_CURRENT_FUNCTION);
}

void RendererOpenGL::beginScene()
{
	// enable alpha blending
	//glEnable(GL_BLEND); // TODO
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//glViewport(xPos, yPos, width, height);
	//glScissor(xPos, yPos, width, height);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//clearFrameBuffer(getCurrentRenderTarget(), { 0.5, 0.5, 0.5, 1 }, 1.0f, 0);

	//glStencilMask(0x00);

	//checkGLErrors(BOOST_CURRENT_FUNCTION);
}

void RendererOpenGL::blitRenderTargets(BaseRenderTarget* src, BaseRenderTarget* dest, const Dimension& dim, int renderComponents)
{
	BaseRenderTargetGL* srcGL = dynamic_cast<BaseRenderTargetGL*>(src);
	BaseRenderTargetGL* destGL = dynamic_cast<BaseRenderTargetGL*>(dest);
	assert(srcGL && destGL);
	//copy the content from the source buffer to the destination buffered
	//if (srcGL->width > destGL->width || srcGL->height > destGL->height) {
	//	LOG(logClient, Warning) << "dest dimension are smaller than the dimension of src!";
	//}

	int componentsGL = getRenderComponentsGL(renderComponents);
	destGL->copyFrom(srcGL, dim, componentsGL);
}

void RendererOpenGL::clearRenderTarget(BaseRenderTarget * renderTarget, int renderComponents)
{
	BaseRenderTargetGL& renderTargetGL = dynamic_cast<BaseRenderTargetGL&>(*renderTarget);
	glBindFramebuffer(GL_FRAMEBUFFER, renderTargetGL.getFrameBuffer());
	int renderComponentsComponentsGL = getRenderComponentsGL(renderComponents);
	glClear(renderComponentsComponentsGL);
}

CubeDepthMap* RendererOpenGL::createCubeDepthMap(int width, int height)
{
	cubeDepthMaps.emplace_back(move(CubeDepthMapGL(width, height)));
	return &cubeDepthMaps.back();
}

CubeRenderTarget * RendererOpenGL::createCubeRenderTarget(int width, int height, const TextureData& data)
{
	CubeRenderTargetGL result(width, height, data);

	cubeRenderTargets.push_back(move(result));
	return &cubeRenderTargets.back();
}

GLint RendererOpenGL::getCurrentRenderTarget() const
{
	GLint drawFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
	return drawFboId;
}

BaseRenderTarget * RendererOpenGL::getDefaultRenderTarget()
{
	return &defaultRenderTarget;
}

RenderTarget* RendererOpenGL::create2DRenderTarget(int width, int height, const TextureData& data, int samples) {

	return createRenderTargetGL(width, height, data, samples, GL_DEPTH_STENCIL); //GL_RGBA
}

void RendererOpenGL::clearFrameBuffer(GLuint frameBuffer, vec4 color, float depthValue, int stencilValue)
{
	// backup current bound drawing frame buffer
	GLint drawFboId = getCurrentRenderTarget();

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	//glClearColor(color.r, color.g, color.b, color.a);

	glViewport(xPos, yPos, width, height);
	glScissor(xPos, yPos, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClearDepth(depthValue);
	glClearStencil(stencilValue);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	// restore frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
}

DepthMap* RendererOpenGL::createDepthMap(int width, int height)
{
	depthMaps.push_back(move(DepthMapGL(width, height)));
	return &depthMaps.back();
}

RenderTarget* RendererOpenGL::createRenderTarget(int samples)
{
	TextureData data;
	data.useSRGB = false;
	data.generateMipMaps = false;
	data.minFilter = Linear;
	data.magFilter = Linear;
	data.colorspace = RGB;
	data.resolution = BITS_32;
	data.uvTechnique = ClampToEdge;
	data.isFloatData = true;

	int ssaaSamples = 1;
	
	//return createRenderTargetGL(width * ssaaSamples, height * ssaaSamples, data, samples, GL_DEPTH_STENCIL); //GL_RGBA
	return createRenderTargetGL(width * ssaaSamples, height * ssaaSamples, data, samples, GL_DEPTH32F_STENCIL8); //GL_RGBA
}

void RendererOpenGL::enableAlphaBlending(bool enable)
{
	if (!enable) {
		glDisable(GL_BLEND);
		return;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RendererOpenGL::enableBackfaceDrawing(bool enable)
{
	if (enable)
	{
		glDisable(GL_CULL_FACE);
	} else
	{
		glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
	}
}

void RendererOpenGL::enableDepthWriting(bool enable)
{
	if (enable)
	{
		glDepthMask(GL_TRUE);
	} else
	{
		glDepthMask(GL_FALSE);
	}
}

void RendererOpenGL::endScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDisable(GL_POLYGON_OFFSET_FILL);
    //checkGLErrors(BOOST_CURRENT_FUNCTION);
}

ModelDrawer* RendererOpenGL::getModelDrawer()
{
	return &modelDrawer;
}

ModelManager* RendererOpenGL::getModelManager()
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

ShaderManager* RendererOpenGL::getShaderManager()
{
	return ShaderManagerGL::get();
}

ShadingModelFactory& RendererOpenGL::getShadingModelFactory()
{
	return *shadingModelFactory.get();
}

SMAA* RendererOpenGL::getSMAA()
{
	return smaa.get();
}

TextureManager* RendererOpenGL::getTextureManager()
{
	return TextureManagerGL::get();
}

RendererType RendererOpenGL::getType() const
{
	return OPENGL;
}

void RendererOpenGL::present()
{
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
		RenderTargetGL& target = *it;
		target.release();
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
	RenderBackend::setViewPort(x, y, width, height);

	this->width = width;
	this->height = height;

	glViewport(xPos, yPos, width, height);
	glScissor(xPos, yPos, width, height);
	defaultRenderTarget = BaseRenderTargetGL(width, height, 0);
	//LOG(logClient, Debug) << "set view port called: " << width << ", " << height;

	//if (effectLibrary)
	//	effectLibrary->getGaussianBlur()->init();
}

void RendererOpenGL::useCubeDepthMap(CubeDepthMap* cubeDepthMap)
{
	CubeDepthMapGL* map = dynamic_cast<CubeDepthMapGL*>(cubeDepthMap);
	assert(map != nullptr);
	CubeMapGL* cubeMap = dynamic_cast<CubeMapGL*>(map->getCubeMap());

	glViewport(xPos, yPos, map->getWidth(), map->getHeight());
	glScissor(xPos, yPos, map->getWidth(), map->getHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, map->getFramebuffer());
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeMap->getCubeMap(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
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

void RendererOpenGL::useCubeRenderTarget(CubeRenderTarget * target, CubeMap::Side side, unsigned int mipLevel)
{
	CubeRenderTargetGL* targetGL = dynamic_cast<CubeRenderTargetGL*>(target);
	assert(targetGL != nullptr);


	CubeMap* cubeMap = target->getCubeMap();
	CubeMapGL* cubeMapGL = dynamic_cast<CubeMapGL*>(cubeMap);

	GLuint AXIS_SIDE = CubeMapGL::mapCubeSideToSystemAxis(side);

	int width = target->getWidth();
	int height = target->getHeight();
	GLuint cubeMapTexture = cubeMapGL->getCubeMap();

	glBindFramebuffer(GL_FRAMEBUFFER, targetGL->getFrameBuffer());

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, AXIS_SIDE, cubeMapTexture, mipLevel);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RendererOpenGL::useBaseRenderTarget(BaseRenderTarget * target)
{
	BaseRenderTargetGL* targetGL = dynamic_cast<BaseRenderTargetGL*>(target);
	assert(targetGL != nullptr);
	int width = targetGL->getWidth();
	int height = targetGL->getHeight();
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, targetGL->getFrameBuffer());

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

void RendererOpenGL::useVarianceShadowMap(RenderTarget* source)
{
	RenderTargetGL* map = dynamic_cast<RenderTargetGL*>(source);
	assert(map != nullptr);
	TextureGL* textureGL = static_cast<TextureGL*>(map->getTexture());

	glViewport(0, 0, map->getWidth(), map->getHeight());
	glScissor(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, map->getFrameBuffer());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RendererOpenGL::checkGLErrors(string errorPrefix)
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

		stringstream ss; ss << move(errorPrefix) << ": Error occured: " << error;
		throw OpenglException(ss.str());
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

void RendererOpenGL::clearRenderTarget(RenderTargetGL* renderTarget, bool releasedAllocatedMemory)
{
	if (releasedAllocatedMemory && renderTarget->frameBuffer != GL_FALSE)
	{
		glDeleteFramebuffers(1, &renderTarget->frameBuffer);
		glDeleteTextures(1, &renderTarget->renderBuffer);
		glDeleteRenderbuffers(1, &renderTarget->textureBuffer.textureID);
	}

	renderTarget->frameBuffer = GL_FALSE;
	renderTarget->renderBuffer = GL_FALSE;
	renderTarget->textureBuffer.setTexture(GL_FALSE);
}

CubeRenderTarget* RendererOpenGL::renderCubeMap(int width, int height, Texture* equirectangularMap)
{
	EquirectangularSkyBoxShader* shader = dynamic_cast<EquirectangularSkyBoxShader*>(getShaderManager()->getConfig(Shaders::SkyBoxEquirectangular));
	shader->setSkyTexture(equirectangularMap);
	Vob skyBox ("misc/SkyBoxCube.obj", Shaders::BlinnPhongTex);

	TextureData textureData = {false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32};


	CubeRenderTargetGL*  result = dynamic_cast<CubeRenderTargetGL*>(createCubeRenderTarget(width, height, std::move(textureData)));


	TransformData data;
	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
	data.projection = &projection;
	mat4 model = glm::mat4();
	data.model = &model;

	//view matrices;
	const mat4 views[] = {
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_X), //right; sign of up vector is not important
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_X), //left
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_Y), //top
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_Y), //bottom
		CubeMap::getViewLookAtMatrixRH(CubeMap::POSITIVE_Z), //back
		CubeMap::getViewLookAtMatrixRH(CubeMap::NEGATIVE_Z) //front
	};


	//set the viewport to the dimensoion of the cubemap
	glViewport(0,0, width, height);
	glScissor(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, result->getFrameBuffer());

	for (int i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result->getCubeMapGL(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//render into the texture
		data.view = &views[i];
		modelDrawer.draw(&skyBox, Shaders::SkyBoxEquirectangular, data);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//register and return the cubemap
	return result;
}

RenderTargetGL* RendererOpenGL::createRenderTargetGL(int width, int height, const TextureData& data,
	GLuint samples, GLuint depthStencilType)
{
	assert(samples >= 1);

	RenderTargetGL result(width, height);

	if (samples > 1)
	{
		result = RenderTargetGL::createMultisampled(width, height, data, samples, depthStencilType);
	}
	else
	{
		result = RenderTargetGL::createSingleSampled(width, height, data, depthStencilType);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	renderTargets.push_back(move(result));
	return &renderTargets.back();
}

std::unique_ptr<SSAO_Deferred> RendererOpenGL::createDeferredSSAO()
{
	return std::make_unique<SSAO_DeferredGL>(width, height, &modelDrawer);
}

std::unique_ptr<hbao::HBAO> RendererOpenGL::createHBAO()
{
	return std::make_unique<hbao::HBAO_GL>(width, height, &modelDrawer);
}

EffectLibrary* RendererOpenGL::getEffectLibrary()
{
	return effectLibrary.get();
}

RenderTarget* RendererOpenGL::createVarianceShadowMap(int width, int height)
{
	RenderTargetGL target = RenderTargetGL::createVSM(width, height);
	renderTargets.push_back(move(target));
	return &renderTargets.back();
}

void RendererOpenGL::cullFaces(CullingMode mode)
{
	if (mode == CullingMode::Front)
	{
		glCullFace(GL_FRONT);

		// TODO this is needed for rendering shadow maps => put it on a more suitable place
		//glEnable(GL_POLYGON_OFFSET_FILL);
		//glPolygonOffset(-1.0f, -1.0f);
	} else
	{
		//glDisable(GL_POLYGON_OFFSET_FILL);
		glCullFace(GL_BACK);
	}
}

void RendererOpenGL::destroyCubeRenderTarget(CubeRenderTarget * target)
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

void RendererOpenGL::destroyRenderTarget(RenderTarget* target)
{
	RenderTargetGL* targetGL = dynamic_cast<RenderTargetGL*>(target);
	assert(targetGL != nullptr);

	for (auto it = renderTargets.begin(); it != renderTargets.end(); ++it)
	{
		if (&(*it) == targetGL)
		{
			targetGL->release();
			renderTargets.erase(it);
			break;
		}
	}
}