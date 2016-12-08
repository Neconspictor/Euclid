#include <model/opengl/ModelManagerGL.hpp>
#include <GL/glew.h>
#include <mesh/TestMeshes.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <exception/MeshNotFoundException.hpp>
#include <sstream>
#include <model/opengl/ModelGL.hpp>
#include <model/opengl/AssimpModelLoader.hpp>

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
		//stringstream ss;
		//ss << "MeshManagerGL::getModel(const std::string&): Model not found: " << modelName;
		//throw MeshNotFoundException(ss.str());
		shared_ptr<ModelGL> model = make_shared<ModelGL>(assimpLoader.loadModel(modelName));
		models[modelName] = model;
		return model.get();
	}

	return dynamic_cast<Model*>(it->second.get());
}


Model* ModelManagerGL::getPositionNormalTexCube()
{
	auto it = models.find(TestMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
	if (it != models.end())
	{
		return it->second.get();
	}

	vector<Mesh::Vertex> vertices;
	unsigned int vertexCount = sizeof(TestMeshes::cubePositionNormalTexVertices) / sizeof(Mesh::Vertex);
	unsigned int vertexSlice = sizeof(Mesh::Vertex) / sizeof(float);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		Mesh::Vertex vertex;
		const float* source = &TestMeshes::cubePositionNormalTexVertices[i * vertexSlice];
		vertex.position = {*(source), *(source + 1), *(source + 2)};
		source += 3;
		vertex.normal = { *(source), *(source + 1), *(source + 2) };
		source += 3;
		vertex.texCoords = { *(source), *(source + 1) };
		vertices.push_back(vertex);
	}

	vector<unsigned int> indices;
	unsigned int indexCount = sizeof(TestMeshes::cubePositionNormalTexIndices) / sizeof(unsigned int);
	for (unsigned int i = 0; i < indexCount; ++i)
	{
		indices.push_back(TestMeshes::cubePositionNormalTexIndices[i]);
	}

	MeshGL mesh = MeshGL(move(vertices), move(indices));
	Material& material = *mesh.getMaterial();
	material.setDiffuseMap("container.png");
	material.setEmissionMap("matrix.jpg");
	material.setSpecularMap("container_s.png");
	material.setShininess(32);

	shared_ptr<ModelGL> model = make_shared<ModelGL>(vector<MeshGL>({ mesh }));
	models[TestMeshes::CUBE_POSITION_NORMAL_TEX_NAME] = model;

	return model.get();
}

ModelManagerGL* ModelManagerGL::get()
{
	return instance.get();
}

void ModelManagerGL::loadModels()
{
	//TODO
	getPositionNormalTexCube();
	//AssimpModelLoader::loadModel("");
}

ModelManagerGL::ModelManagerGL()
{
}