#include <model/opengl/ModelManagerGL.hpp>
#include <mesh/TestMeshes.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <model/opengl/ModelGL.hpp>
#include <model/opengl/AssimpModelLoader.hpp>
#include <texture/opengl/TextureManagerGL.hpp>

using namespace std;

unique_ptr<ModelManagerGL> ModelManagerGL::instance = make_unique<ModelManagerGL>(ModelManagerGL());

ModelManagerGL::~ModelManagerGL()
{
}

ModelGL ModelManagerGL::createSpriteModel(float xPos, float yPos, float widthWeight, float heightWeight)
{
	// create a Quad mesh that fills up the enter screen; screen space range from [-1, 1] in x,y and z axis;
	// as we want a  2D model, the z axis is ignored/set to 0.0f
	// normal vectors aren't needed, too -> set to 0.0f as well.
	vector<Mesh::Vertex> vertices;
	Mesh::Vertex vertex;

	// height values grow from top to bottom (screen space: 1.0f to -1.0f)
	heightWeight = - 2 * heightWeight;

	// width goes from left to right; screen space: -1.0f to 1.0f
	widthWeight = 2 * widthWeight;

	// xPos goes from left to right; screen space: -1.0f to 1.0f
	xPos = 2 * xPos - 1.0f;

	// yPos goes from top to bottom; screen space: 1.0f to -1.0f
	yPos = -2 * yPos + 1.0f;
	
	// left upper corner 
	vertex.position = { xPos,  yPos, 0.0f};
	vertex.normal = { 0.0f, 0.0f, 0.0f };
	vertex.texCoords = {0.0f, 1.0f};
	vertices.push_back(vertex);

	// left bottom corner
	vertex.position = { xPos,  yPos + heightWeight, 0.0f };
	vertex.normal = { 0.0f, 0.0f, 0.0f };
	vertex.texCoords = { 0.0f, 0.0f };
	vertices.push_back(vertex);

	// right bottom corner
	vertex.position = { xPos + widthWeight,  yPos + heightWeight, 0.0f };
	vertex.normal = { 0.0f, 0.0f, 0.0f };
	vertex.texCoords = { 1.0f, 0.0f };
	vertices.push_back(vertex);

	// right upper corner
	vertex.position = { xPos + widthWeight,  yPos, 0.0f };
	vertex.normal = { 0.0f, 0.0f, 0.0f };
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

	MeshGL mesh = MeshGL(move(vertices), move(indices));
	ModelGL model(vector<MeshGL>{mesh});
	return model;
}

Model* ModelManagerGL::getModel(const string& modelName)
{
	auto it = models.find(modelName);
	if (it == models.end())
	{
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
	TextureManagerGL* textureManager = TextureManagerGL::get();
	material.setDiffuseMap(textureManager->getImage("container.png"));
	material.setEmissionMap(textureManager->getImage("matrix.jpg"));
	material.setSpecularMap(textureManager->getImage("container_s.png"));
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