#include <mesh/opengl/MeshManagerGL.hpp>
#include <GL/glew.h>
#include <mesh/TestMeshes.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <exception/MeshNotFoundException.hpp>
#include <sstream>
#include <model/opengl/ModelGL.hpp>

using namespace std;

unique_ptr<ModelManagerGL> ModelManagerGL::instance = make_unique<ModelManagerGL>(ModelManagerGL());

ModelManagerGL::~ModelManagerGL()
{
}

Model* ModelManagerGL::getModel(const string& modelName)
{
	auto it = models.find(modelName);
	if (it == models.end())
	{
		stringstream ss;
		ss << "MeshManagerGL::getModel(const std::string&): Model not found: " << modelName;
		throw MeshNotFoundException(ss.str());
	}

	return dynamic_cast<Model*>(it->second.get());
}

Model* ModelManagerGL::getPositionCube()
{
	auto it = models.find(TestMeshes::CUBE_POSITION_NAME);
	if (it != models.end())
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

	vector<float> vertices;
	for (int i = 0; i < sizeof(TestMeshes::cubePositionVertices) / sizeof(float); ++i)
	{
		vertices.push_back(TestMeshes::cubePositionVertices[i]);
	}

	MeshGL mesh = MeshGL(move(vertices), TestMeshes::CUBE_POSITION_VERTEX_SLICE, vector<size_t>(),
		VAO, VBO, vertexCount);
	shared_ptr<ModelGL> model = make_shared<ModelGL>(ModelGL(vector<MeshGL>{ mesh }));
	models[TestMeshes::CUBE_POSITION_NAME] = model;
	
	return model.get();
}

Model* ModelManagerGL::getPositionNormalCube()
{
	auto it = models.find(TestMeshes::CUBE_POSITION_NORMAL_NAME);
	if (it != models.end())
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

	vector<float> vertices;
	for (int i = 0; i < sizeof(TestMeshes::cubePositionNormalVertices) / sizeof(float); ++i)
	{
		vertices.push_back(TestMeshes::cubePositionNormalVertices[i]);
	}

	MeshGL mesh = MeshGL(move(vertices), TestMeshes::CUBE_POSITION_NORMAL_VERTEX_SLICE, vector<size_t>(),
		VAO, VBO, vertexCount);
	shared_ptr<ModelGL> model = make_shared<ModelGL>(vector<MeshGL>({ mesh }));
	models[TestMeshes::CUBE_POSITION_NORMAL_NAME] = model;

	return model.get();
}

Model* ModelManagerGL::getPositionNormalTexCube()
{
	auto it = models.find(TestMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
	if (it != models.end())
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

	vector<float> vertices;
	for (int i = 0; i < sizeof(TestMeshes::cubePositionNormalTexVertices) / sizeof(float); ++i)
	{
		vertices.push_back(TestMeshes::cubePositionNormalTexVertices[i]);
	}

	MeshGL mesh = MeshGL(move(vertices), TestMeshes::CUBE_POSITION_NORMAL_TEX_VERTEX_SLICE, vector<size_t>(),
		VAO, VBO, vertexCount);
	shared_ptr<ModelGL> model = make_shared<ModelGL>(vector<MeshGL>({ mesh }));
	models[TestMeshes::CUBE_POSITION_NORMAL_TEX_NAME] = model;

	return model.get();
}

Model* ModelManagerGL::getTexturedCube()
{

	auto it = models.find(TestMeshes::CUBE_POSITION_UV_NAME);
	if (it != models.end())
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

	vector<float> vertices;
	for (int i = 0; i < sizeof(TestMeshes::cubePositionUVVertices) / sizeof(float); ++i)
	{
		vertices.push_back(TestMeshes::cubePositionUVVertices[i]);
	}

	MeshGL mesh = MeshGL(move(vertices), TestMeshes::CUBE_POSITION_UV_VERTEX_SLICE,
		vector<size_t>(),
		VAO, VBO, vertexCount);
	shared_ptr<ModelGL> model = make_shared<ModelGL>(vector<MeshGL>({ mesh }));
	models[TestMeshes::CUBE_POSITION_UV_NAME] = model;

	return model.get();
}

ModelManagerGL* ModelManagerGL::get()
{
	return instance.get();
}

void ModelManagerGL::loadModels()
{
	//TODO
	getTexturedCube();
	getPositionCube();
	getPositionNormalCube();
	getPositionNormalTexCube();
}

ModelManagerGL::ModelManagerGL()
{
}