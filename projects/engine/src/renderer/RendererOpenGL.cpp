#include "renderer/RendererOpenGL.hpp"
#include <iostream>
#include <GL/glew.h>
#include <GL/GLU.h>

using namespace std;

RendererOpenGL::RendererOpenGL()
{
}

RendererOpenGL::~RendererOpenGL()
{
}

void RendererOpenGL::init()
{
	cout << "RendererOpenGL::init() called." << endl;
	glViewport(0, 0, 800, 600);
	int aspectratio = 800 / 600;
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

RendererType RendererOpenGL::getType()
{
	return OPENGL;
}
