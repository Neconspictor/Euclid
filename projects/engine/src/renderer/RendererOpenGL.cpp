#include <renderer/RendererOpenGL.hpp>
#include <GL/glew.h>
#include <GL/GLU.h>

using namespace std;
using namespace platform;

RendererOpenGL::RendererOpenGL() : Renderer()
{
	logClient.setPrefix("[RendererOpenGL]");
}

RendererOpenGL::~RendererOpenGL()
{
}

void RendererOpenGL::init()
{
	LOG(logClient, Info) << "Initializing...";
	glViewport(xPos, yPos, width, height);
	int aspectratio = width / height;
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	
	gluPerspective(45.0f, aspectratio, 0.2f, 255.0f);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();


	glClearColor(1.0f, 1.0f, 0.0f, 1.0f); // White Background
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void RendererOpenGL::beginScene()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f); // Dark greyish Background
	glClear(GL_COLOR_BUFFER_BIT);
}

void RendererOpenGL::endScene()
{
}

void RendererOpenGL::present()
{
}

void RendererOpenGL::release()
{
}

RendererType RendererOpenGL::getType() const
{
	return OPENGL;
}