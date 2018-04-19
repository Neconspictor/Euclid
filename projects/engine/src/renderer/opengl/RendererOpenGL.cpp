#include <renderer/opengl/RendererOpenGL.hpp>
#include <shader/opengl/ShaderManagerGL.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <platform/exception/OpenglException.hpp>
#include <model/opengl/ModelManagerGL.hpp>
#include <drawing/opengl/ModelDrawerGL.hpp>
#include <shader/opengl/ScreenShaderGL.hpp>
#include <glad/glad.h>
#include <GL/gl.h>
#include <antialiasing/opengl/SMAA_GL.hpp>
#include <fstream>
#include <texture/opengl/ImageLoaderGL.hpp>
#include<post_processing/blur/opengl/GaussianBlurGL.hpp>

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
	gaussianBlur->release();
}

RendererOpenGL::RendererOpenGL() : Renderer3D(), 
  screenSprite(nullptr), backgroundColor(0.0f, 0.0f, 0.0f), modelDrawer(this),
  msaaSamples(1), smaa(nullptr)
{	
	logClient.setPrefix("[RendererOpenGL]");

	//clearRenderTarget(&singleSampledScreenBuffer, false);
	//clearRenderTarget(&multiSampledScreenBuffer, false);

	RendererOpenGL* renderer = this;

	smaa = new SMAA_GL(renderer);
}

RendererOpenGL::~RendererOpenGL()
{
	delete smaa;
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

	glViewport(xPos, yPos, width, height);

	checkGLErrors(BOOST_CURRENT_FUNCTION);
	createFrameRenderTargetBuffer(width, height);
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	glClearColor(0.1f, 0.1f, 0.1f, 0.1f); // White Background
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do

	// stencil buffering is enabled when needed!
	//glEnable(GL_STENCIL_TEST); // Enable stencil buffering

	// we want counter clock wise winding order
	glFrontFace(GL_CCW);

	// only draw front faces
	enableBackfaceDrawing(false);

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

	checkGLErrors(BOOST_CURRENT_FUNCTION);
}

void RendererOpenGL::beginScene()
{
	//glViewport(xPos, yPos, width, height);
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do

	// stencil buffering is enabled when needed!
	//glEnable(GL_STENCIL_TEST); // Enable stencil buffering

	// we want counter clock wise winding order
	glFrontFace(GL_CCW);

	// only draw front faces
	//enableBackfaceDrawing(false);

	// enable alpha blending
	//glEnable(GL_BLEND); // TODO
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f); // Dark greyish Background
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//clearFrameBuffer(getCurrentRenderTarget(), { backgroundColor.r, backgroundColor.g, backgroundColor.b, 1 }, 1.0f, 0);

	glStencilMask(0x00);

	//glDisable(GL_MULTISAMPLE);
	glEnable(GL_MULTISAMPLE);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	enableDepthWriting(true);

	checkGLErrors(BOOST_CURRENT_FUNCTION);
}

void RendererOpenGL::blitRenderTargets(RenderTarget* src, RenderTarget* dest)
{
	RenderTargetGL* srcGL = dynamic_cast<RenderTargetGL*>(src);
	RenderTargetGL* destGL = dynamic_cast<RenderTargetGL*>(dest);
	assert(srcGL && destGL);
	//copy the content from the source buffer to the destination buffered
	if (srcGL->width > destGL->width || srcGL->height > destGL->height) {
		LOG(logClient, Warning) << "dest dimension are smaller than the dimension of src!";
	}
	Dimension dim = {0, 0, srcGL->width, srcGL->height};
	destGL->copyFrom(srcGL, dim, dim);
}

CubeDepthMap* RendererOpenGL::createCubeDepthMap(int width, int height)
{
	cubeDepthMaps.push_back(move(CubeDepthMapGL(width, height)));
	return &cubeDepthMaps.back();
}

GLint RendererOpenGL::getCurrentRenderTarget() const
{
	GLint drawFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
	return drawFboId;
}

void RendererOpenGL::clearFrameBuffer(GLuint frameBuffer, vec4 color, float depthValue, int stencilValue)
{
	// backup current bound drawing frame buffer
	GLint drawFboId = getCurrentRenderTarget();

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glClearColor(color.r, color.g, color.b, color.a);
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
	RenderTargetGL target = createRenderTargetGL_intern(GL_RGBA32F, width, height, samples, GL_DEPTH_STENCIL); //GL_RGBA
	renderTargets.push_back(move(target));
	return &renderTargets.back();
}

RenderTargetGL* RendererOpenGL::createRenderTargetGL(GLint textureChannel, int width, int height, GLuint samples, GLuint depthStencilType)
{
	RenderTargetGL target = createRenderTargetGL_intern(textureChannel, width, height, samples, depthStencilType);
	renderTargets.push_back(move(target));
	return &renderTargets.back();
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
		glCullFace(GL_BACK);
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
	glDisable(GL_POLYGON_OFFSET_FILL);
    checkGLErrors(BOOST_CURRENT_FUNCTION);
}

ModelDrawer* RendererOpenGL::getModelDrawer()
{
	return &modelDrawer;
}

ModelManager* RendererOpenGL::getModelManager()
{
	return ModelManagerGL::get();
}

ShaderManager* RendererOpenGL::getShaderManager()
{
	return ShaderManagerGL::get();
}

SMAA* RendererOpenGL::getSMAA()
{
	return smaa;
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
	effectLibrary->release();
	renderTargets.clear();
	depthMaps.clear();
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
	Renderer::setViewPort(x, y, width, height);
	glViewport(xPos, yPos, width, height);
	LOG(logClient, Debug) << "set view port called: " << width << ", " << height;

	//if (singleSampledScreenBuffer.getFrameBuffer() == GL_FALSE) return;
	// update offscreen buffer texture
	createFrameRenderTargetBuffer(width, height);

	if (effectLibrary)
		effectLibrary->getGaussianBlur()->init();
}

void RendererOpenGL::useCubeDepthMap(CubeDepthMap* cubeDepthMap)
{
	CubeDepthMapGL* map = dynamic_cast<CubeDepthMapGL*>(cubeDepthMap);
	assert(map != nullptr);
	CubeMapGL* cubeMap = dynamic_cast<CubeMapGL*>(map->getCubeMap());

	glViewport(xPos, yPos, map->getWidth(), map->getHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, map->getFramebuffer());
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeMap->getCubeMap(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void RendererOpenGL::useDepthMap(DepthMap* depthMap)
{
	DepthMapGL* map = dynamic_cast<DepthMapGL*>(depthMap);
	assert(map != nullptr);
	TextureGL* textureGL = static_cast<TextureGL*>(map->getTexture());

	glViewport(xPos, yPos, map->getWidth(), map->getHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, map->getFramebuffer());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureGL->getTexture(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0f, 0.0f);

	glClear(GL_DEPTH_BUFFER_BIT);
}

void RendererOpenGL::useRenderTarget(RenderTarget* target)
{
	RenderTargetGL* targetGL = dynamic_cast<RenderTargetGL*>(target);
	assert(targetGL != nullptr);
	glViewport(0, 0, targetGL->width, targetGL->height);
	glBindFramebuffer(GL_FRAMEBUFFER, targetGL->getFrameBuffer());
	//clearFrameBuffer(targetGL->getFrameBuffer(), { 0, 0, 0, 1 }, 1.0f, 0);
}

void RendererOpenGL::useScreenTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// clear the stencil (with 1.0) and depth (with 0) buffer of the screen buffer 
	glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
	clearFrameBuffer(0, { 0.5, 0.5, 0.5, 1 }, 1.0f, 0);
}

void RendererOpenGL::useVarianceShadowMap(RenderTarget* source)
{
	RenderTargetGL* map = dynamic_cast<RenderTargetGL*>(source);
	assert(map != nullptr);
	TextureGL* textureGL = static_cast<TextureGL*>(map->getTexture());

	glViewport(xPos, yPos, map->getWidth(), map->getHeight());
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

void RendererOpenGL::createFrameRenderTargetBuffer(int width, int height)
{
	// created a screen buffer once before? -> release it
	//clearRenderTarget(&singleSampledScreenBuffer);
	//clearRenderTarget(&multiSampledScreenBuffer);

	//createSingleSampledScreenBuffer(&singleSampledScreenBuffer);
	//singleSampledScreenBuffer = createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
	//multiSampledScreenBuffer = createRenderTarget(GL_RGBA8, width, height, msaaSamples, GL_DEPTH_STENCIL);

	checkGLErrors(BOOST_CURRENT_FUNCTION);
}

RenderTargetGL RendererOpenGL::createRenderTargetGL_intern(GLint textureChannel, int width, int height,
	GLuint samples, GLuint depthStencilType) const
{
	assert(samples >= 1);

	RenderTargetGL result(width, height);

	if (samples > 1)
	{
		result = RenderTargetGL::createMultisampled(textureChannel, width, height, samples, depthStencilType);
	}
	else
	{
		result = RenderTargetGL::createSingleSampled(textureChannel, width, height, depthStencilType);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	return result;
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
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		// TODO this is needed for rendering shadow maps => put it on a more suitable place
		//glEnable(GL_POLYGON_OFFSET_FILL);
		//glPolygonOffset(-2.0f, 0.0f);
	} else
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
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