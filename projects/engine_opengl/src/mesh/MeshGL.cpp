#include <mesh/MeshGL.hpp>

using namespace std;

MeshGL::MeshGL() : vao(GL_FALSE), vbo(GL_FALSE), ebo(GL_FALSE)
{
}

MeshGL::~MeshGL()
{
	// TODO should not be needed
	/*GLuint vertexArrayObject = this->getVertexArrayObject();
	this->setVertexArrayObject(GL_FALSE);
	glDeleteVertexArrays(1, &vertexArrayObject);

	GLuint vertexBufferObject = this->getVertexBufferObject();
	this->setVertexBufferObject(GL_FALSE);
	glDeleteBuffers(1, &vertexBufferObject);*/
}

GLuint MeshGL::getVertexArrayObject() const
{
	return vao;
}

GLuint MeshGL::getVertexBufferObject() const
{
	return vbo;
}

GLuint MeshGL::getElementBufferObject() const
{
	return ebo;
}

void MeshGL::setVertexArrayObject(GLuint vao)
{
	this->vao = vao;
}

void MeshGL::setVertexBufferObject(GLuint vbo)
{
	this->vbo = vbo;
}

void MeshGL::setElementBufferObject(GLuint ebo)
{
	this->ebo = ebo;
}