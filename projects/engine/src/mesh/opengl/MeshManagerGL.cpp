#include <mesh/opengl/MeshManagerGL.hpp>
#include <GL/glew.h>
#include <mesh/TestMeshes.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <exception/MeshNotFoundException.hpp>
#include <sstream>
#include <iostream>

using namespace std;

unique_ptr<MeshManagerGL> MeshManagerGL::instance = make_unique<MeshManagerGL>(MeshManagerGL());

MeshManagerGL::~MeshManagerGL()
{
}

Mesh* MeshManagerGL::getMesh(const string& meshName)
{
	auto it = meshes.find(meshName);
	if (it == meshes.end())
	{
		stringstream ss;
		ss << "MeshManagerGL::getMesh(const std::string&): Mesh not found: " << meshName;
		throw MeshNotFoundException(ss.str());
	}

	return it->second.get();
}

Mesh* MeshManagerGL::getSimpleLitCube()
{
	auto it = meshes.find(TestMeshes::CUBE_SIMPLE_LIT_NAME);
	if (it != meshes.end())
	{
		return it->second.get();
	}

	GLuint VAO, VBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);

	// 1. Bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. Copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int vertexCount = sizeof(TestMeshes::cubeVertices) / sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TestMeshes::cubeVertices), TestMeshes::cubeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shared_ptr<MeshGL> mesh = make_shared<MeshGL>(VAO, VBO, vertexCount);
	//shared_ptr<MeshGL> mesh = shared_ptr<MeshGL>(new MeshGL(VAO, VBO, vertexCount), meshRelease);
	meshes[TestMeshes::CUBE_SIMPLE_LIT_NAME] = mesh;
	return mesh.get();
}

Mesh* MeshManagerGL::getTexturedCube()
{

	auto it = meshes.find(TestMeshes::CUBE_NAME);
	if (it != meshes.end())
	{
		return it->second.get();
	}

	GLuint VAO, VBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);

	// 1. Bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. Copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int vertexCount = sizeof(TestMeshes::cubeVertices) / sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TestMeshes::cubeVertices), TestMeshes::cubeVertices, GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// color
	//GLuint EBO;
	//glGenBuffers(1, &EBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangleIndices), rectangleIndices, GL_STATIC_DRAW);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(1);

	// uv coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//5. Unbind the VAO
	glBindVertexArray(0);

	shared_ptr<MeshGL> mesh = make_shared<MeshGL>(VAO, VBO, vertexCount);
	meshes[TestMeshes::CUBE_NAME] = mesh;
	return mesh.get();
}

MeshManagerGL* MeshManagerGL::get()
{
	return instance.get();
}

void MeshManagerGL::loadMeshes()
{
	//TODO
}

MeshManagerGL::MeshManagerGL()
{
}

void MeshManagerGL::meshRelease(MeshGL* mesh)
{
	GLuint vertexArrayObject = mesh->getVertexArrayObject();
	mesh->setVertexArrayObject(GL_FALSE);
	glDeleteVertexArrays(1, &vertexArrayObject);

	GLuint vertexBufferObject = mesh->getVertexBufferObject();
	mesh->setVertexBufferObject(GL_FALSE);
	glDeleteBuffers(1, &vertexBufferObject);
}