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

using namespace std;
using namespace platform;
using namespace glm;

RendererOpenGL::RendererOpenGL() : Renderer3D(), 
screenSprite(nullptr), backgroundColor(0.0f, 0.0f, 0.0f), msaaSamples(1), smaa(nullptr)
{	
	logClient.setPrefix("[RendererOpenGL]");

	clearRenderTarget(&singleSampledScreenBuffer, false);
	clearRenderTarget(&multiSampledScreenBuffer, false);

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

	ModelGL* screenSpritePtr = static_cast<ModelGL*>
		(ModelManagerGL::get()->createSpriteModel(0.0f, 0.0f, 1.0f, 1.0f));
	screenSprite.reset(screenSpritePtr);

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

	smaa->init();

	checkGLErrors(BOOST_CURRENT_FUNCTION);

}

void RendererOpenGL::beginScene()
{
	glViewport(xPos, yPos, width, height);
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

	checkGLErrors(BOOST_CURRENT_FUNCTION);
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

void RendererOpenGL::drawOffscreenBuffer()
{
	ScreenShaderGL* shaderGL = static_cast<ScreenShaderGL*>(
									ShaderManagerGL::get()->getShader(Screen));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//glLineWidth(3);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	shaderGL->setOffscreenBuffer(singleSampledScreenBuffer.textureBuffer);
	for (Mesh* mesh : screenSprite->getMeshes())
	{
		shaderGL->draw(*mesh);
	}
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

	if (singleSampledScreenBuffer.frameBuffer == GL_FALSE) return;
	// update offscreen buffer texture
	createFrameRenderTargetBuffer(width, height);
}

void RendererOpenGL::useOffscreenBuffer()
{
	//glBindFramebuffer(GL_FRAMEBUFFER, singleSampledScreenBuffer.frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, multiSampledScreenBuffer.frameBuffer);
	//int color = 0;
	//float depth = 1.0f;
	//int stencil = 0;
	//glClearBufferiv(GL_COLOR, 0, &color);
	//glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
	clearFrameBuffer(singleSampledScreenBuffer.frameBuffer, { 0, 0, 0, 1 }, 1.0f, 0);
}

void RendererOpenGL::useScreenBuffer()
{
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multiSampledScreenBuffer.frameBuffer);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singleSampledScreenBuffer.frameBuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multiSampledScreenBuffer.frameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singleSampledScreenBuffer.frameBuffer);

	glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
	//clearFrameBuffer(singleSampledScreenBuffer.frameBuffer, { 0.5, 0.5, 0.5, 1 }, 1.0f, 0);

	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//int color = 0;
	//float depth = 1.0f;
	//int stencil = 0;
	//glClearBufferiv(GL_COLOR, 0, &color);
	//glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
	clearFrameBuffer(0, { 0.5, 0.5, 0.5, 1 }, 1.0f, 0);
}

void RendererOpenGL::checkGLErrors(string errorPrefix) const
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
		glDeleteTextures(1, &renderTarget->textureBuffer);
		glDeleteRenderbuffers(1, &renderTarget->renderBuffer);
	}

	renderTarget->frameBuffer = GL_FALSE;
	renderTarget->textureBuffer = GL_FALSE;
	renderTarget->renderBuffer = GL_FALSE;
}

void RendererOpenGL::createFrameRenderTargetBuffer(int width, int height)
{
	// created a screen buffer once before? -> release it
	clearRenderTarget(&singleSampledScreenBuffer);
	clearRenderTarget(&multiSampledScreenBuffer);

	//createSingleSampledScreenBuffer(&singleSampledScreenBuffer);
	singleSampledScreenBuffer = createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
	multiSampledScreenBuffer = createRenderTarget(GL_RGBA8, width, height, msaaSamples, GL_DEPTH_STENCIL);

	checkGLErrors(BOOST_CURRENT_FUNCTION);
}

RendererOpenGL::RenderTargetGL RendererOpenGL::createRenderTarget(GLint textureChannel, int width, int height, 
	GLuint samples, GLuint depthStencilType) const
{
	assert(samples >= 1);

	RenderTargetGL result;
	glGenFramebuffers(1, &result.frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, result.frameBuffer);

	if (samples > 1)
	{
		// Generate texture
		glGenTextures(1, &result.textureBuffer);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, result.textureBuffer);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, textureChannel, width, height, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		// attach texture to currently bound frame buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, result.textureBuffer, 0);

		//create a render buffer for depth and stencil testing
		glGenRenderbuffers(1, &result.renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, result.renderBuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, depthStencilType, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	} else
	{
		// Generate texture
		glGenTextures(1, &result.textureBuffer);
		glBindTexture(GL_TEXTURE_2D, result.textureBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, textureChannel, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// clamp is important so that no pixel artifacts occur on the border!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);

		// attach texture to currently bound frame buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result.textureBuffer, 0);

		//create a render buffer for depth and stencil testing
		glGenRenderbuffers(1, &result.renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, result.renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, depthStencilType, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	// attach render buffer to the frame buffer
	if (depthStencilType == GL_DEPTH_COMPONENT)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
	} else
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
	}

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw runtime_error("RendererOpenGL::createRenderTarget(): Couldn't successfully init framebuffer!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	return result;
}

void RendererOpenGL::destroyRenderTarget(RenderTargetGL* renderTarget) const
{
	glDeleteFramebuffers(1, &renderTarget->frameBuffer);
	glDeleteTextures(1, &renderTarget->textureBuffer);
	glDeleteRenderbuffers(1, &renderTarget->renderBuffer);

	renderTarget->frameBuffer = GL_FALSE;
	renderTarget->textureBuffer = GL_FALSE;
	renderTarget->renderBuffer = GL_FALSE;
}