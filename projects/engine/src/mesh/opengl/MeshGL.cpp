#include <mesh/opengl/MeshGL.hpp>

using namespace std;

MeshGL::MeshGL(vector<Vertex> vertices, vector<unsigned int> indices) : Mesh(vertices, indices)
{
	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo);
	glCreateBuffers(1, &ebo);

	// 1. bind Vertex Array Object
	glBindVertexArray(vao);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), &vertexData[0], GL_STATIC_DRAW);
	// 3. copy our indixes in a buffer, too.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), &indexData[0], GL_STATIC_DRAW);

	// vertex attribute pointers
	// vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<GLvoid*>(0));

	// vertex normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, normal)));

	// vertex uv coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, texCoords)));

	// clear opengl states
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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

void MeshGL::setVertexArrayObject(GLuint vao)
{
	this->vao = vao;
}

void MeshGL::setVertexBufferObject(GLuint vbo)
{
	this->vbo = vbo;
}