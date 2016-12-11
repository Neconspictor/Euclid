#include <renderer/RendererOpenGL.hpp>
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

RendererOpenGL::RendererOpenGL() : Renderer3D(), offscreenFrameBuffer(0), texColorBuffer(0), screenSprite({})
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

	screenSprite =  ModelManagerGL::createScreenSpriteModel();
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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	checkGLErrors(BOOST_CURRENT_FUNCTION);

}

void RendererOpenGL::beginScene()
{
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // Dark greyish Background
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
	for (Mesh* mesh : screenSprite.getMeshes())
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

void RendererOpenGL::setViewPort(int x, int y, int width, int height)
{
	Renderer::setViewPort(x, y, width, height);
	glViewport(xPos, yPos, width, height);
}

void RendererOpenGL::useOffscreenBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, offscreenFrameBuffer);
	beginScene();
}

void RendererOpenGL::useScreenBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	beginScene();
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