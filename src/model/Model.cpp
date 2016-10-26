#include <model/Model.hpp>
#include <iostream>

using namespace glm;

Model::Model(GLuint vertexArrayObject, GLuint vertexBufferObject, unsigned int vertexCount) :
	vao(vertexArrayObject), vbo(vertexBufferObject), vertexCount(vertexCount)
{
}

Model::~Model()
{
	std::cout << "~Model() called!" << std::endl;
}

GLuint Model::getVertexArrayObject() const
{
	return vao;
}

GLuint Model::getVertexBufferObject() const
{
	return vbo;
}

mat4 const& Model::getModelMatrix() const
{
	return modelMatrix;
}

unsigned Model::getVertexCount() const
{
	return vertexCount;
}

void Model::setModelMatrix(mat4&& matrix)
{
	modelMatrix = matrix;
}

void Model::setModelMatrix(mat4 const& matrix)
{
	modelMatrix = mat4(matrix);
}

void Model::setVertexArrayObject(GLuint vao)
{
	this->vao = vao;
}

void Model::setVertexBufferObject(GLuint vbo)
{
	this->vbo = vbo;
}