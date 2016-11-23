#include <mesh/opengl/MeshGL.hpp>

using namespace std;

MeshGL::MeshGL(GLuint vertexArrayObject, GLuint vertexBufferObject, unsigned int vertexCount) :
	vao(vertexArrayObject), vbo(vertexBufferObject), vertexCount(vertexCount)
{
}

MeshGL::~MeshGL()
{
	GLuint vertexArrayObject = this->getVertexArrayObject();
	this->setVertexArrayObject(GL_FALSE);
	glDeleteVertexArrays(1, &vertexArrayObject);

	GLuint vertexBufferObject = this->getVertexBufferObject();
	this->setVertexBufferObject(GL_FALSE);
	glDeleteBuffers(1, &vertexBufferObject);
}

GLuint MeshGL::getVertexArrayObject() const
{
	return vao;
}

GLuint MeshGL::getVertexBufferObject() const
{
	return vbo;
}

unsigned MeshGL::getVertexCount() const
{
	return vertexCount;
}

void MeshGL::setVertexArrayObject(GLuint vao)
{
	this->vao = vao;
}

void MeshGL::setVertexBufferObject(GLuint vbo)
{
	this->vbo = vbo;
}