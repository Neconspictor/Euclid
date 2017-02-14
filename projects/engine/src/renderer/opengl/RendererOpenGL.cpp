﻿#include <renderer/opengl/RendererOpenGL.hpp>
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

using namespace std;
using namespace platform;
using namespace glm;

RendererOpenGL::RendererOpenGL() : Renderer3D(), 
screenSprite(nullptr), backgroundColor(0.0f, 0.0f, 0.0f), msaaSamples(1), smaa(nullptr)
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

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White Background
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do

	// stencil buffering is enabled when needed!
	//glEnable(GL_STENCIL_TEST); // Enable stencil buffering

	// we want counter clock wise winding order
	glFrontFace(GL_CCW);

	// only draw front faces
	enableBackfaceDrawing(false);

	// enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	checkGLErrors(BOOST_CURRENT_FUNCTION);

	ShaderGL::initShaderFileSystem();

	checkGLErrors(BOOST_CURRENT_FUNCTION);

	smaa->init();

	ImageLoaderGL imageLoader;
	GenericImageGL image = imageLoader.loadImageFromDisc("testImage.dds");
	if (image.pixels)
	{
		delete[] image.pixels;
		image.pixels = nullptr;
	}

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
	enableBackfaceDrawing(false);

	// enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f); // Dark greyish Background
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	clearFrameBuffer(getCurrentRenderTarget(), { backgroundColor.r, backgroundColor.g, backgroundColor.b, 1 }, 1.0f, 0);

	glStencilMask(0x00);

	//glDisable(GL_MULTISAMPLE);
	glEnable(GL_MULTISAMPLE);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	checkGLErrors(BOOST_CURRENT_FUNCTION);
}

void RendererOpenGL::blitRenderTargets(RenderTarget* src, RenderTarget* dest)
{
	RenderTargetGL* srcGL = dynamic_cast<RenderTargetGL*>(src);
	RenderTargetGL* destGL = dynamic_cast<RenderTargetGL*>(dest);
	assert(srcGL && destGL, " RendererOpenGL::blitRenderTargets(RenderTarget*, RenderTarget*): Couldn't cast src and dest to RenderTargetGL objects!");
	//copy the content from the source buffer to the destination buffered
	Dimension dim = {xPos, yPos, width, height};
	srcGL->copyFrom(destGL, dim, dim);
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
	RenderTargetGL target = createRenderTarget(GL_RGBA, width, height, samples, GL_DEPTH_STENCIL);
	renderTargets.push_back(move(target));
	return &renderTargets.back();
}

/*void RendererOpenGL::drawOffscreenBuffer()
{
	ScreenShaderGL* shaderGL = static_cast<ScreenShaderGL*>(
									ShaderManagerGL::get()->getShader(Screen));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//glLineWidth(3);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	shaderGL->setOffscreenBuffer(singleSampledScreenBuffer.getTextureGL());
	for (Mesh* mesh : screenSprite->getMeshes())
	{
		shaderGL->draw(*mesh);
	}
}*/

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
    checkGLErrors(BOOST_CURRENT_FUNCTION);
}

ModelDrawer* RendererOpenGL::getModelDrawer()
{
	return ModelDrawerGL::get();
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
}

void RendererOpenGL::useDepthMap(DepthMap* depthMap)
{
	DepthMapGL* map = static_cast<DepthMapGL*>(depthMap);
	assert(map != nullptr);

	glViewport(xPos, yPos, map->getWidth(), map->getHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, map->getFramebuffer());

	//glClear(GL_DEPTH_BUFFER_BIT); // -> is done in beginScene()
	// glBindFramebuffer(GL_FRAMEBUFFER, 0); // has to be done later - necessary step or can it be left out at all? 
	// glBindTexture(GL_TEXTURE_2D, map->getTexture()) // has to be done by client at a later step
}

void RendererOpenGL::useRenderTarget(RenderTarget* target)
{
	RenderTargetGL* targetGL = dynamic_cast<RenderTargetGL*>(target);
	assert(targetGL != nullptr, "RendererOpenGL::useRenderTarget(RenderTarget*): Couldn't cast to RenderTargetGL!");
	glViewport(xPos, yPos, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, targetGL->getFrameBuffer());
	clearFrameBuffer(targetGL->getFrameBuffer(), { 0, 0, 0, 1 }, 1.0f, 0);
}

void RendererOpenGL::useScreenTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// clear the stencil (with 1.0) and depth (with 0) buffer of the screen buffer 
	glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
	clearFrameBuffer(0, { 0.5, 0.5, 0.5, 1 }, 1.0f, 0);
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

RenderTargetGL RendererOpenGL::createRenderTarget(GLint textureChannel, int width, int height, 
	GLuint samples, GLuint depthStencilType) const
{
	assert(samples >= 1);

	RenderTargetGL result;

	if (samples > 1)
	{
		result = RenderTargetGL::createMultisampled(textureChannel, width, height, samples, depthStencilType);
	} else
	{
		result = RenderTargetGL::createSingleSampled(textureChannel, width, height, depthStencilType);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	return result;
}

void RendererOpenGL::destroyRenderTarget(RenderTarget* target)
{
	RenderTargetGL* targetGL = dynamic_cast<RenderTargetGL*>(target);
	assert(targetGL != nullptr, "RendererOpenGL::destroyRenderTarget(RenderTarget*): Cast to RenderTargetGL failed!");

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