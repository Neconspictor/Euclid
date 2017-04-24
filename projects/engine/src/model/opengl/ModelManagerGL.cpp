#include <model/opengl/ModelManagerGL.hpp>
#include <mesh/SampleMeshes.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <model/opengl/ModelGL.hpp>
#include <model/opengl/AssimpModelLoader.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <mesh/opengl/MeshFactoryGL.hpp>

using namespace std;
using namespace glm;

unique_ptr<ModelManagerGL> ModelManagerGL::instance = make_unique<ModelManagerGL>(ModelManagerGL());

ModelManagerGL::ModelManagerGL()
{
}

ModelManagerGL::~ModelManagerGL()
{
}

Model* ModelManagerGL::getSkyBox()
{
	using Vertex = VertexPosition;

	auto it = modelTable.find(SKYBOX_MODEL_NAME);
	if (it == modelTable.end())
	{
		size_t vertexCount = sizeof(SampleMeshes::skyBoxVertices);
		size_t indexCount = sizeof(SampleMeshes::skyBoxIndices);

		MeshGL mesh = MeshFactoryGL::createPosition((const Vertex*)SampleMeshes::skyBoxVertices, vertexCount,
			SampleMeshes::skyBoxIndices, indexCount);
		models.push_back(ModelGL(vector<MeshGL>{mesh}));
		ModelGL* result = &models.back();
		modelTable[SKYBOX_MODEL_NAME] = result;
		return &models.back();
	}

	return dynamic_cast<Model*>(it->second);
}

Model* ModelManagerGL::getSprite()
{
	using Vertex = VertexPositionTex;

	auto it = modelTable.find(SPRITE_MODEL_NAME);
	if (it != modelTable.end())
	{
		return dynamic_cast<Model*>(it->second);
	}

	// create a Quad mesh that fills up the enter screen; screen space range from [-1, 1] in x,y and z axis;
	// as we want a  2D model, the z axis is ignored/set to 1.0f
	// normal vectors aren't needed, too -> set to 0.0f as well.
	vector<Vertex> vertices;
	Vertex vertex;
	
	// left upper corner 
	vertex.position = { 0.0f,  0.0f, 1.0f};
	vertex.texCoords = {0.0f, 1.0f};
	vertices.push_back(vertex);

	// left bottom corner
	vertex.position = { 0.0f,  1.0, 1.0f };
	vertex.texCoords = { 0.0f, 0.0f };
	vertices.push_back(vertex);

	// right bottom corner
	vertex.position = { 1.0f,  1.0f, 1.0f };
	vertex.texCoords = { 1.0f, 0.0f };
	vertices.push_back(vertex);

	// right upper corner
	vertex.position = { 1.0f,  0.0f, 1.0f };
	vertex.texCoords = { 1.0f, 1.0f };
	vertices.push_back(vertex);

	// create index buffer in counter clock winding order
	vector<unsigned int> indices;

	// first triangle
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(3);

	// second triangle
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);

	MeshGL mesh = MeshFactoryGL::createPositionUV(vertices.data(), vertices.size(), 
										indices.data(), indices.size());

	models.push_back(ModelGL(vector<MeshGL>{mesh}));
	ModelGL* result = &models.back();
	modelTable[SPRITE_MODEL_NAME] = result;
	return &models.back();
}

Model* ModelManagerGL::getModel(const string& modelName)
{
	auto it = modelTable.find(modelName);
	if (it != modelTable.end())
	{
		return dynamic_cast<Model*>(it->second);
	}

	if (modelName.compare(SPRITE_MODEL_NAME) == 0)
	{
		return getSprite();
	}

	if(modelName.compare(SKYBOX_MODEL_NAME) == 0)
	{
		return getSkyBox();
	}

	// else case: assume the model name is a 3d model that can be load from file.
	models.push_back(assimpLoader.loadModel(modelName));
	ModelGL* result = &models.back();
	modelTable[modelName] = result;
	return &models.back();
}


Model* ModelManagerGL::getPositionNormalTexCube()
{
	using Vertex = VertexPositionNormalTex;

	auto it = modelTable.find(SampleMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
	if (it != modelTable.end())
	{
		return it->second;
	}

	vector<Vertex> vertices;
	unsigned int vertexCount = sizeof(SampleMeshes::cubePositionNormalTexVertices) / sizeof(Vertex);
	unsigned int vertexSlice = sizeof(Vertex) / sizeof(float);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		Vertex vertex;
		const float* source = &SampleMeshes::cubePositionNormalTexVertices[i * vertexSlice];
		vertex.position = {*(source), *(source + 1), *(source + 2)};
		source += 3;
		vertex.normal = { *(source), *(source + 1), *(source + 2) };
		source += 3;
		vertex.texCoords = { *(source), *(source + 1) };
	}

	vector<unsigned int> indices;
	unsigned int indexCount = sizeof(SampleMeshes::cubePositionNormalTexIndices) / sizeof(unsigned int);
	for (unsigned int i = 0; i < indexCount; ++i)
	{
		indices.push_back(SampleMeshes::cubePositionNormalTexIndices[i]);
	}

	MeshGL mesh = MeshFactoryGL::create(vertices.data(), vertices.size(), 
										indices.data(), indices.size());
	Material& material = *mesh.getMaterial();
	TextureManagerGL* textureManager = TextureManagerGL::get();
	material.setDiffuseMap(textureManager->getImage("container.png"));
	material.setEmissionMap(textureManager->getImage("matrix.jpg"));
	material.setSpecularMap(textureManager->getImage("container_s.png"));
	material.setShininess(32);

	models.push_back(ModelGL(vector<MeshGL>({ mesh })));
	modelTable[SampleMeshes::CUBE_POSITION_NORMAL_TEX_NAME] = &models.back();

	return &models.back();
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

void ModelManagerGL::useInstances(Model* source, mat4* modelMatrices, unsigned int amount)
{
	ModelGL* model = static_cast<ModelGL*>(source);
	model->createInstanced(amount, modelMatrices);
}