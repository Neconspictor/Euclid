#include <model/ModelFactory.hpp>
#include <model/BasicModel.hpp>
#include <mesh/TestMeshes.hpp>

using namespace std;
using namespace glm;
using namespace TestMeshes;

shared_ptr<Model> ModelFactory::texturedCube()
{
	GLuint VAO, VBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);

	// 1. Bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. Copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int vertexCount = sizeof(cubeVertices) / sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

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

	Model* model = new Model(VAO, VBO, vertexCount);
	return shared_ptr<Model>(model, modelRelease);
}

shared_ptr<Model> ModelFactory::simpleLitCube()
{
	GLuint VAO, VBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);

	// 1. Bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. Copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int vertexCount = sizeof(cubeVertices) / sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Model* model = new Model(VAO, VBO, vertexCount);
	return shared_ptr<Model>(model, modelRelease);
}

void ModelFactory::modelRelease(Model* model)
{
	GLuint vertexArrayObject = model->getVertexArrayObject();
	model->setVertexArrayObject(GL_FALSE);
	glDeleteVertexArrays(1, &vertexArrayObject);

	GLuint vertexBufferObject = model->getVertexBufferObject();
	model->setVertexBufferObject(GL_FALSE);
	glDeleteBuffers(1, &vertexBufferObject);
}