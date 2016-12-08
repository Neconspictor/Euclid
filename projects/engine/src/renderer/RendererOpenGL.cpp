#include <renderer/RendererOpenGL.hpp>
#include <GL/glew.h>
#include <GL/GLU.h>
#include <shader/opengl/ShaderManagerGL.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <platform/exception/OpenglException.hpp>
#include <model/opengl/ModelManagerGL.hpp>

using namespace std;
using namespace platform;

RendererOpenGL::RendererOpenGL() : Renderer3D()
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

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White Background
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do

	glGenVertexArrays(1, &vertexArrayObjID);
	glBindVertexArray(vertexArrayObjID);
	// This will identify our vertex buffer
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);
	glEnableVertexAttribArray(0);

	checkGLErrors(BOOST_CURRENT_FUNCTION);

}

void RendererOpenGL::beginScene()
{
	glClearColor(0.7f, 0.7f, 1.0f, 1.0f); // Dark greyish Background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	checkGLErrors(BOOST_CURRENT_FUNCTION);
}



void RendererOpenGL::endScene()
{
	// 1rst attribute buffer : vertices
	/*glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

	// Draw the triangle !
	glBindVertexArray(vertexArrayObjID); // First VAO
	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);*/

	checkGLErrors(BOOST_CURRENT_FUNCTION);
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