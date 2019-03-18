#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <nex/material/BlinnPhongMaterialLoader.hpp>
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


const unsigned int nex::StaticMeshManager::SKYBOX_MODEL_HASH = nex::util::customSimpleHash("_INTERN_MODELS__SKYBOX");
	const unsigned int nex::StaticMeshManager::SPRITE_MODEL_HASH = nex::util::customSimpleHash("_INTERN_MODELS__SPRITE");


	nex::StaticMeshManager::StaticMeshManager() :
		pbrMaterialLoader(TextureManager::get()),
		blinnPhongMaterialLoader(TextureManager::get()), mFileSystem(nullptr)
	{
		CUBE_POSITION_NORMAL_TEX_HASH = nex::util::customSimpleHash(nex::sample_meshes::CUBE_POSITION_NORMAL_TEX_NAME);

		mFullscreenPlane = std::make_unique<VertexArray>();
		static const float fullscreenPlaneTriangleStripVerticesOpengl[] = {
			// position 4 floats, texture coords 2 floats
			-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
			+1.0, -1.0, 0.0, 1.0, 1.0, 0.0,
			-1.0, +1.0, 0.0, 1.0, 0.0, 1.0,
			+1.0, +1.0, 0.0, 1.0, 1.0, 1.0
		};

		VertexBuffer planeBuffer(fullscreenPlaneTriangleStripVerticesOpengl, sizeof(fullscreenPlaneTriangleStripVerticesOpengl));
		VertexLayout layout;
		layout.push<float>(4);
		layout.push<float>(2);
		mFullscreenPlane->addBuffer(std::move(planeBuffer), layout);


		mFullscreenTriangle = std::make_unique<VertexArray>();
		static const float fullscreenTriangleVerticesOpengl[] = {
			// position 4 floats, texture coords 2 floats
			-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
			+2.0, -1.0, 0.0, 1.0, 2.0, 0.0,
			-1.0, +2.0, 0.0, 1.0, 0.0, 2.0
		};

		VertexBuffer triangleBuffer(fullscreenTriangleVerticesOpengl, sizeof(fullscreenTriangleVerticesOpengl));
		mFullscreenTriangle->addBuffer(std::move(triangleBuffer), layout);
	}

std::unique_ptr<nex::StaticMesh> nex::StaticMeshManager::createSphere(unsigned xSegments, unsigned ySegments,
	std::unique_ptr<Material> material)
{
	std::vector<std::unique_ptr<SubMesh>> meshes;
	std::vector<std::unique_ptr<Material>> materials;
	materials.emplace_back(std::move(material));
	meshes.emplace_back(std::make_unique<Sphere>(xSegments, ySegments, materials.back().get()));
	return std::make_unique<StaticMesh>(std::move(meshes), std::move(materials));
}

nex::StaticMesh* nex::StaticMeshManager::getSkyBox()
	{
		using Vertex = VertexPosition;

		using namespace nex::util;

		auto it = modelTable.find(SKYBOX_MODEL_HASH);
		if (it == modelTable.end())
		{
			int vertexCount = (int)sizeof(sample_meshes::skyBoxVertices);
			int indexCount = (int)sizeof(sample_meshes::skyBoxIndices);

			std::unique_ptr<SubMesh> mesh = MeshFactory::createPosition((const Vertex*)sample_meshes::skyBoxVertices, vertexCount,
				sample_meshes::skyBoxIndices, (int)indexCount);

			std::vector<std::unique_ptr<SubMesh>> meshes;
			meshes.push_back(move(mesh));

			auto model = std::make_unique<StaticMesh>(move(meshes), std::vector<std::unique_ptr<Material>>());

			models.push_back(move(model));
			StaticMesh* result = models.back().get();
			modelTable[SKYBOX_MODEL_HASH] = result;
			return result;
		}

		return it->second;
	}

	nex::StaticMesh* nex::StaticMeshManager::getSprite()
	{
		using Vertex = VertexPositionTex;

		auto it = modelTable.find(SPRITE_MODEL_HASH);
		if (it != modelTable.end())
		{
			return dynamic_cast<StaticMesh*>(it->second);
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

		std::unique_ptr<SubMesh> mesh = MeshFactory::createPositionUV(vertices.data(), (int)vertices.size(),
			indices.data(), (int)indices.size());


		std::vector<std::unique_ptr<SubMesh>> meshes;
		meshes.push_back(move(mesh));

		auto model = std::make_unique<StaticMesh>(move(meshes), std::vector<std::unique_ptr<Material>>());

		models.push_back(std::move(model));

		StaticMesh* result = models.back().get();
		modelTable[SPRITE_MODEL_HASH] = result;
		return result;
	}

	void nex::StaticMeshManager::init(FileSystem* meshFileSystem)
	{
		mFileSystem = meshFileSystem;
	}

	nex::StaticMesh* nex::StaticMeshManager::getModel(const std::string& meshPath, nex::MaterialType type)
	{
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

		// else case: assume the model name is a 3d model that can be load from file.

		const auto resolvedPath = mFileSystem->resolvePath(meshPath);

		nex::AbstractMaterialLoader* materialLoader = nullptr;
		if (type == MaterialType::BlinnPhong) {
			materialLoader = &blinnPhongMaterialLoader;
		}
		else if (type == MaterialType::Pbr) {
			materialLoader = &pbrMaterialLoader;
		}
		else {
			std::stringstream msg;
			msg << "No suitable material loader found for shader type: " << type << std::endl; //TODO

			throw_with_trace(std::runtime_error(msg.str()));
		}



		models.push_back(move(assimpLoader.loadStaticMesh(resolvedPath, *materialLoader)));
		StaticMesh* result = models.back().get();
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


nex::StaticMesh* nex::StaticMeshManager::getPositionNormalTexCube()
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

		std::unique_ptr<SubMesh> mesh = MeshFactory::create(vertices.data(), (int)vertices.size(),
			indices.data(), (int)indices.size());

		std::vector<std::unique_ptr<SubMesh>> meshes;
		meshes.push_back(move(mesh));

		auto model = std::make_unique<StaticMesh>(move(meshes), std::vector<std::unique_ptr<Material>>());

		models.push_back(std::move(model));

		StaticMesh* result = models.back().get();
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
	}

	/*void ModelManagerGL::useInstances(ModelGL* source, mat4* modelMatrices, unsigned int amount)
	{
		ModelGL* model = static_cast<ModelGL*>(source);
		model->createInstanced(amount, modelMatrices);
	}*/