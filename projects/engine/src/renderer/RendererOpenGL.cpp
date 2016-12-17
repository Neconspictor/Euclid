﻿#include <renderer/RendererOpenGL.hpp>
#include <shader/opengl/ShaderManagerGL.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <platform/exception/OpenglException.hpp>
#include <model/opengl/ModelManagerGL.hpp>
#include <drawing/opengl/ModelDrawerGL.hpp>
#include <GL/glew.h>
#include <GL/GLU.h>
#include <shader/opengl/ScreenShaderGL.hpp>

using namespace std;
using namespace platform;

RendererOpenGL::RendererOpenGL() : Renderer3D(), offscreenFrameBuffer(0), texColorBuffer(0), screenSprite(nullptr),
	backgroundColor(0.0f, 0.0f, 0.0f)
{
	logClient.setPrefix("[RendererOpenGL]");
}

RendererOpenGL::~RendererOpenGL()
{
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

	screenSprite = static_cast<ModelGL*>(
						ModelManagerGL::get()->createSpriteModel(0.35f,0.0f, 0.3f, 0.3f));
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

}

void RendererOpenGL::beginScene()
{
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f); // Dark greyish Background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);

	checkGLErrors(BOOST_CURRENT_FUNCTION);
}

void RendererOpenGL::drawOffscreenBuffer()
{
	ScreenShaderGL* shaderGL = static_cast<ScreenShaderGL*>(
									ShaderManagerGL::get()->getShader(Screen));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//glLineWidth(3);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	shaderGL->setOffscreenBuffer(texColorBuffer);
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

void RendererOpenGL::setViewPort(int x, int y, int width, int height)
{
	Renderer::setViewPort(x, y, width, height);
	glViewport(xPos, yPos, width, height);
}

void RendererOpenGL::useOffscreenBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, offscreenFrameBuffer);
}

void RendererOpenGL::useScreenBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RendererOpenGL::checkGLErrors(string errorPrefix) const
{
	// check if any gl related errors occured
	GLint error = glGetError();
	if (error != GL_NO_ERROR)
	{
		stringstream ss; ss << move(errorPrefix) << ": Error occured: " << gluErrorString(error);
		throw OpenglException(ss.str());
	}
}

void RendererOpenGL::createFrameRenderTargetBuffer(int width, int height)
{
	glGenFramebuffers(1, &offscreenFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, offscreenFrameBuffer);
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	// Generate texture
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// clamp is important so that no pixel artifacts occur on the border!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBindTexture(GL_TEXTURE_2D, 0);
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	// attach texture to currently bound frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	//create a render buffer for depth and stencil testing
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	checkGLErrors(BOOST_CURRENT_FUNCTION);

	// attach render buffer to the frame buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw runtime_error("RendererOpenGL::createFrameRenderTargetBuffer(): Couldn't successfully init framebuffer!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkGLErrors(BOOST_CURRENT_FUNCTION);
}