#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/mesh/MeshFactory.hpp>
#include <sstream>
#include <string>
#include <nex/util/StringUtils.hpp>
#include <nex/mesh/SampleMeshes.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/mesh/Sphere.hpp>
#include "nex/material/Material.hpp"
#include "nex/material/AbstractMaterialLoader.hpp"
#include <nex/FileSystem.hpp>
#include "VertexLayout.hpp"
#include "VertexBuffer.hpp"


	nex::StaticMeshManager::StaticMeshManager() :
		mFileSystem(nullptr),
		mInitialized(false)
	{
		CUBE_POSITION_NORMAL_TEX_HASH = nex::util::customSimpleHash(sample_meshes::CUBE_POSITION_NORMAL_TEX_NAME);
		SKYBOX_MODEL_HASH = nex::util::customSimpleHash(sample_meshes::SKY_BOX_NAME);
		SPRITE_MODEL_HASH = nex::util::customSimpleHash(sample_meshes::RECTANGLE_NAME);

		mFullscreenPlane = std::make_unique<VertexArray>();
		static const float fullscreenPlaneTriangleStripVerticesOpengl[] = {
			// position 4 floats, texture coords 2 floats
			-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
			+1.0, -1.0, 0.0, 1.0, 1.0, 0.0,
			-1.0, +1.0, 0.0, 1.0, 0.0, 1.0,
			+1.0, +1.0, 0.0, 1.0, 1.0, 1.0
		};

		mFullscreenTriangleData = std::make_unique<VertexBuffer>(fullscreenPlaneTriangleStripVerticesOpengl, sizeof(fullscreenPlaneTriangleStripVerticesOpengl));
		VertexLayout layout;
		layout.push<float>(4);
		layout.push<float>(2);
		mFullscreenPlane->bind();
		mFullscreenPlane->useBuffer(*mFullscreenTriangleData, layout);
		mFullscreenPlane->unbind(); // important: In OpenGL implementation VertexBuffer creation with arguments corrupts state of vertex array, if not unbounded!


		mFullscreenTriangle = std::make_unique<VertexArray>();
		static const float fullscreenTriangleVerticesOpengl[] = {
			// position 4 floats, texture coords 2 floats
			-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
			+3.0, -1.0, 0.0, 1.0, 2.0, 0.0,
			-1.0, +3.0, 0.0, 1.0, 0.0, 2.0
		};

		mFullscreenTriangleData = std::make_unique<VertexBuffer>(fullscreenTriangleVerticesOpengl, sizeof(fullscreenTriangleVerticesOpengl));
		mFullscreenTriangle->bind();
		mFullscreenTriangle->useBuffer(*mFullscreenTriangleData, layout);
		mFullscreenTriangle->unbind();
	}

std::unique_ptr<nex::StaticMeshContainer> nex::StaticMeshManager::createSphere(unsigned xSegments, unsigned ySegments,
	std::unique_ptr<Material> material)
{
	auto mesh = std::make_unique<Sphere>(xSegments, ySegments);
	auto model = std::make_unique<StaticMeshContainer>();
	model->add(std::move(mesh), std::move(material));

	return model;
}

nex::StaticMeshContainer* nex::StaticMeshManager::getSkyBox()
	{
		using Vertex = VertexPosition;

		using namespace nex::util;

		auto it = modelTable.find(SKYBOX_MODEL_HASH);
		if (it == modelTable.end())
		{
			int vertexCount = (int)sizeof(sample_meshes::skyBoxVertices);
			int indexCount = (int)sizeof(sample_meshes::skyBoxIndices);

			std::unique_ptr<Mesh> mesh = MeshFactory::createPosition((const Vertex*)sample_meshes::skyBoxVertices, vertexCount,
				sample_meshes::skyBoxIndices, (int)indexCount);

			auto model = std::make_unique<StaticMeshContainer>();
			model->add(std::move(mesh), nullptr);

			models.push_back(move(model));
			StaticMeshContainer* result = models.back().get();
			modelTable[SKYBOX_MODEL_HASH] = result;
			return result;
		}

		return it->second;
	}

	nex::StaticMeshContainer* nex::StaticMeshManager::getSprite()
	{
		using Vertex = VertexPositionTex;

		auto it = modelTable.find(SPRITE_MODEL_HASH);
		if (it != modelTable.end())
		{
			return dynamic_cast<StaticMeshContainer*>(it->second);
		}

		// create a Quad mesh that fills up the enter screen; normalized device coordinates range from [-1, 1] in x,y and z axis;
		// as we want a  2D model, the z axis is ignored/set to 1.0f
		// normal vectors aren't needed, too -> set to 0.0f as well.
		std::vector<Vertex> vertices;
		Vertex vertex;

		// left upper corner 
		vertex.position = { 0.0f,  0.0f, 0.0f };
		vertex.texCoords = { 0.0f, 1.0f };
		vertices.push_back(vertex);

		// left bottom corner
		vertex.position = { 0.0f,  1.0, 0.0f };
		vertex.texCoords = { 0.0f, 0.0f };
		vertices.push_back(vertex);

		// right bottom corner
		vertex.position = { 1.0f,  1.0f, 0.0f };
		vertex.texCoords = { 1.0f, 0.0f };
		vertices.push_back(vertex);

		// right upper corner
		vertex.position = { 1.0f,  0.0f, 0.0f };
		vertex.texCoords = { 1.0f, 1.0f };
		vertices.push_back(vertex);

		// create index buffer in counter clock winding order
		std::vector<unsigned int> indices;

		// first triangle
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(3);

		// second triangle
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(3);

		std::unique_ptr<Mesh> mesh = MeshFactory::createPositionUV(vertices.data(), (int)vertices.size(),
			indices.data(), (int)indices.size());

		auto model = std::make_unique<StaticMeshContainer>();
		model->add(std::move(mesh), nullptr);

		models.push_back(std::move(model));

		StaticMeshContainer* result = models.back().get();
		modelTable[SPRITE_MODEL_HASH] = result;
		return result;
	}

	void nex::StaticMeshManager::init(FileSystem* meshFileSystem, std::unique_ptr<PbrMaterialLoader> pbrMaterialLoader)
	{
		//if (pbrMaterialLoader == nullptr) throw std::runtime_error("pbr material loader mustn't be null!");
		mFileSystem = meshFileSystem;
		mPbrMaterialLoader = std::move(pbrMaterialLoader);
		mDefaultMaterialLoader = std::make_unique<DefaultMaterialLoader>();
		mInitialized = true;
	}

	nex::StaticMeshContainer* nex::StaticMeshManager::getModel(const std::string& meshPath, nex::MaterialType type)
	{
		// else case: assume the model name is a 3d model that can be load from file.
		if (!mInitialized) throw std::runtime_error("StaticMeshManager isn't initialized!");

		auto hash = nex::util::customSimpleHash(meshPath);

		auto it = modelTable.find(hash);
		if (it != modelTable.end())
		{
			return it->second;
		}

		if (hash == SPRITE_MODEL_HASH)
		{
			return getSprite();
		}

		if (hash == SKYBOX_MODEL_HASH)
		{
			return getSkyBox();
		}

		const auto resolvedPath = mFileSystem->resolvePath(meshPath);

		nex::AbstractMaterialLoader* materialLoader = nullptr;
		if (type == MaterialType::Pbr) {
			materialLoader = mPbrMaterialLoader.get();
		} else if (type == MaterialType::None)
		{
			materialLoader = mDefaultMaterialLoader.get();
		}
		
		if (materialLoader == nullptr) 
		{
			std::stringstream msg;
			msg << "No suitable material loader found for shader type: " << type << std::endl; //TODO

			throw_with_trace(std::runtime_error(msg.str()));
		}



		models.push_back(move(assimpLoader.loadStaticMesh(resolvedPath, *materialLoader)));
		StaticMeshContainer* result = models.back().get();
		modelTable[hash] = result;
		return result;
	}

nex::VertexArray* nex::StaticMeshManager::getNDCFullscreenPlane()
{
	return mFullscreenPlane.get();
}

nex::VertexArray* nex::StaticMeshManager::getNDCFullscreenTriangle()
{
	return mFullscreenTriangle.get();
}


nex::StaticMeshContainer* nex::StaticMeshManager::getPositionNormalTexCube()
	{
		using Vertex = VertexPositionNormalTex;

		auto it = modelTable.find(CUBE_POSITION_NORMAL_TEX_HASH);
		if (it != modelTable.end())
		{
			return it->second;
		}

		std::vector<Vertex> vertices;
		unsigned int vertexCount = sizeof(sample_meshes::cubePositionNormalTexVertices) / sizeof(Vertex);
		unsigned int vertexSlice = sizeof(Vertex) / sizeof(float);
		for (unsigned int i = 0; i < vertexCount; ++i)
		{
			Vertex vertex;
			const float* source = &sample_meshes::cubePositionNormalTexVertices[i * vertexSlice];
			vertex.position = { *(source), *(source + 1), *(source + 2) };
			source += 3;
			vertex.normal = { *(source), *(source + 1), *(source + 2) };
			source += 3;
			vertex.texCoords = { *(source), *(source + 1) };
			vertices.emplace_back(std::move(vertex));
		}

		std::vector<unsigned int> indices;
		unsigned int indexCount = sizeof(sample_meshes::cubePositionNormalTexIndices) / sizeof(unsigned int);
		for (unsigned int i = 0; i < indexCount; ++i)
		{
			indices.push_back(sample_meshes::cubePositionNormalTexIndices[i]);
		}

		std::unique_ptr<Mesh> mesh = MeshFactory::create(vertices.data(), (int)vertices.size(),
			indices.data(), (int)indices.size());


		auto model = std::make_unique<StaticMeshContainer>();
		model->add(std::move(mesh), nullptr);

		models.push_back(std::move(model));

		StaticMeshContainer* result = models.back().get();
		modelTable[CUBE_POSITION_NORMAL_TEX_HASH] = result;

		return result;
	}

	nex::StaticMeshManager* nex::StaticMeshManager::get()
	{
		static StaticMeshManager instance;
		return &instance;
	}

	void nex::StaticMeshManager::loadModels()
	{
		//TODO
		getPositionNormalTexCube();


		//AssimpModelLoader::loadModel("");
	}

	void nex::StaticMeshManager::release()
	{
		modelTable.clear();
		models.clear();
		mFullscreenPlane.reset(nullptr);
		mFullscreenTriangle.reset(nullptr);
		mFullscreenPlaneData.reset(nullptr);
		mFullscreenTriangleData.reset(nullptr);
	}

void nex::StaticMeshManager::setPbrMaterialLoader(std::unique_ptr<PbrMaterialLoader> pbrMaterialLoader)
{
	mPbrMaterialLoader = std::move(pbrMaterialLoader);
}

/*void ModelManagerGL::useInstances(ModelGL* source, mat4* modelMatrices, unsigned int amount)
	{
		ModelGL* model = static_cast<ModelGL*>(source);
		model->createInstanced(amount, modelMatrices);
	}*/
