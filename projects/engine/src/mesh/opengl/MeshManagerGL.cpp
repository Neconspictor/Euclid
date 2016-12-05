#include <mesh/opengl/MeshManagerGL.hpp>
#include <GL/glew.h>
#include <mesh/TestMeshes.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <exception/MeshNotFoundException.hpp>
#include <sstream>

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

Mesh* MeshManagerGL::getPositionCube()
{
	auto it = meshes.find(TestMeshes::CUBE_POSITION_NAME);
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

	unsigned int vertexCount = sizeof(TestMeshes::cubePositionVertices) / sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TestMeshes::cubePositionVertices), TestMeshes::cubePositionVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, TestMeshes::CUBE_POSITION_VERTEX_SLICE * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shared_ptr<MeshGL> mesh = make_shared<MeshGL>(VAO, VBO, vertexCount);
	//shared_ptr<MeshGL> mesh = shared_ptr<MeshGL>(new MeshGL(VAO, VBO, vertexCount), meshRelease);
	meshes[TestMeshes::CUBE_POSITION_NAME] = mesh;
	return mesh.get();
}

Mesh* MeshManagerGL::getPositionNormalCube()
{
	auto it = meshes.find(TestMeshes::CUBE_POSITION_NORMAL_NAME);
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

	unsigned int vertexCount = sizeof(TestMeshes::cubePositionNormalVertices) / sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TestMeshes::cubePositionNormalVertices), TestMeshes::cubePositionNormalVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, TestMeshes::CUBE_POSITION_NORMAL_VERTEX_SLICE * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// vertex normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, TestMeshes::CUBE_POSITION_NORMAL_VERTEX_SLICE * sizeof(GLfloat),
		(GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shared_ptr<MeshGL> mesh = make_shared<MeshGL>(VAO, VBO, vertexCount);

	meshes[TestMeshes::CUBE_POSITION_NORMAL_NAME] = mesh;
	return mesh.get();
}

Mesh* MeshManagerGL::getPositionNormalTexCube()
{
	auto it = meshes.find(TestMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
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

	unsigned int vertexCount = sizeof(TestMeshes::cubePositionNormalTexVertices) / sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TestMeshes::cubePositionNormalTexVertices), TestMeshes::cubePositionNormalTexVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, TestMeshes::CUBE_POSITION_NORMAL_TEX_VERTEX_SLICE * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// vertex normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, TestMeshes::CUBE_POSITION_NORMAL_TEX_VERTEX_SLICE * sizeof(GLfloat),
		(GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	// vertex normals
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, TestMeshes::CUBE_POSITION_NORMAL_TEX_VERTEX_SLICE * sizeof(GLfloat),
		(GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shared_ptr<MeshGL> mesh = make_shared<MeshGL>(VAO, VBO, vertexCount);

	meshes[TestMeshes::CUBE_POSITION_NORMAL_TEX_NAME] = mesh;
	return mesh.get();
}

Mesh* MeshManagerGL::getTexturedCube()
{

	auto it = meshes.find(TestMeshes::CUBE_POSITION_UV_NAME);
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

	unsigned int vertexCount = sizeof(TestMeshes::cubePositionUVVertices) / sizeof(GLfloat);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TestMeshes::cubePositionUVVertices), TestMeshes::cubePositionUVVertices, GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, TestMeshes::CUBE_POSITION_UV_VERTEX_SLICE * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// uv coords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, TestMeshes::CUBE_POSITION_UV_VERTEX_SLICE * sizeof(GLfloat), 
		(GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// color
	//GLuint EBO;
	//glGenBuffers(1, &EBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangleIndices), rectangleIndices, GL_STATIC_DRAW);
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//5. Unbind the VAO
	glBindVertexArray(0);

	shared_ptr<MeshGL> mesh = make_shared<MeshGL>(VAO, VBO, vertexCount);
	meshes[TestMeshes::CUBE_POSITION_UV_NAME] = mesh;
	return mesh.get();
}

MeshManagerGL* MeshManagerGL::get()
{
	return instance.get();
}

void MeshManagerGL::loadMeshes()
{
	//TODO
	getTexturedCube();
	getPositionCube();
	getPositionNormalCube();
	getPositionNormalTexCube();
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