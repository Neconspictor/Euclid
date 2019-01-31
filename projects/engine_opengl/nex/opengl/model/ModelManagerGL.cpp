#include <nex/opengl/model/ModelManagerGL.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>
#include <nex/opengl/model/ModelGL.hpp>
#include <nex/opengl/model/AssimpModelLoader.hpp>
#include <nex/opengl/material/BlinnPhongMaterialLoader.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/opengl/mesh/MeshFactoryGL.hpp>
#include <sstream>
#include <string>
#include <nex/util/StringUtils.hpp>
#include <nex/opengl/mesh/SampleMeshes.hpp>
#include <nex/shader/Shader.hpp>
#include "nex/opengl/mesh/Sphere.hpp"


const unsigned int nex::ModelManagerGL::SKYBOX_MODEL_HASH = nex::util::customSimpleHash("_INTERN_MODELS__SKYBOX");
	const unsigned int nex::ModelManagerGL::SPRITE_MODEL_HASH = nex::util::customSimpleHash("_INTERN_MODELS__SPRITE");


	nex::ModelManagerGL::ModelManagerGL() :
		pbrMaterialLoader(TextureManagerGL::get()),
		blinnPhongMaterialLoader(TextureManagerGL::get()), mFileSystem(nullptr)
	{
		CUBE_POSITION_NORMAL_TEX_HASH = nex::util::customSimpleHash(nex::sample_meshes::CUBE_POSITION_NORMAL_TEX_NAME);
	}

std::unique_ptr<nex::ModelGL> nex::ModelManagerGL::createSphere(unsigned xSegments, unsigned ySegments,
	std::unique_ptr<Material> material)
{
	std::vector<std::unique_ptr<MeshGL>> meshes;
	std::vector<std::unique_ptr<Material>> materials;
	materials.emplace_back(std::move(material));
	meshes.emplace_back(std::make_unique<Sphere>(xSegments, ySegments, materials.back().get()));
	return std::make_unique<ModelGL>(std::move(meshes), std::move(materials));
}

nex::ModelGL* nex::ModelManagerGL::getSkyBox()
	{
		using Vertex = VertexPosition;

		using namespace nex::util;

		auto it = modelTable.find(SKYBOX_MODEL_HASH);
		if (it == modelTable.end())
		{
			int vertexCount = (int)sizeof(sample_meshes::skyBoxVertices);
			int indexCount = (int)sizeof(sample_meshes::skyBoxIndices);

			std::unique_ptr<MeshGL> mesh = MeshFactoryGL::createPosition((const Vertex*)sample_meshes::skyBoxVertices, vertexCount,
				sample_meshes::skyBoxIndices, (int)indexCount);

			std::vector<std::unique_ptr<MeshGL>> meshes;
			meshes.push_back(move(mesh));

			auto model = std::make_unique<ModelGL>(move(meshes), std::vector<std::unique_ptr<Material>>());

			models.push_back(move(model));
			ModelGL* result = models.back().get();
			modelTable[SKYBOX_MODEL_HASH] = result;
			return result;
		}

		return it->second;
	}

	nex::ModelGL* nex::ModelManagerGL::getSprite()
	{
		using Vertex = VertexPositionTex;

		auto it = modelTable.find(SPRITE_MODEL_HASH);
		if (it != modelTable.end())
		{
			return dynamic_cast<ModelGL*>(it->second);
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

		std::unique_ptr<MeshGL> mesh = MeshFactoryGL::createPositionUV(vertices.data(), (int)vertices.size(),
			indices.data(), (int)indices.size());


		std::vector<std::unique_ptr<MeshGL>> meshes;
		meshes.push_back(move(mesh));

		auto model = std::make_unique<ModelGL>(move(meshes), std::vector<std::unique_ptr<Material>>());

		models.push_back(std::move(model));

		ModelGL* result = models.back().get();
		modelTable[SPRITE_MODEL_HASH] = result;
		return result;
	}

	void nex::ModelManagerGL::init(FileSystem* meshFileSystem)
	{
		mFileSystem = meshFileSystem;
	}

	nex::ModelGL* nex::ModelManagerGL::getModel(const std::string& meshPath, nex::ShaderType materialShader)
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
		if (materialShader == ShaderType::BlinnPhongTex) {
			materialLoader = &blinnPhongMaterialLoader;
		}
		else if (materialShader == ShaderType::Pbr) {
			materialLoader = &pbrMaterialLoader;
		}
		else {
			std::stringstream msg;
			msg << "No suitable material loader found for shader type: " << materialShader << std::endl; //TODO

			throw_with_trace(std::runtime_error(msg.str()));
		}



		models.push_back(move(assimpLoader.loadModel(resolvedPath, *materialLoader)));
		ModelGL* result = models.back().get();
		modelTable[hash] = result;
		return result;
	}


	nex::ModelGL* nex::ModelManagerGL::getPositionNormalTexCube()
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

		std::unique_ptr<MeshGL> mesh = MeshFactoryGL::create(vertices.data(), (int)vertices.size(),
			indices.data(), (int)indices.size());


		BlinnPhongMaterial* material = dynamic_cast<BlinnPhongMaterial*>(mesh->getMaterial());

		if (material) {
			TextureManagerGL* textureManager = TextureManagerGL::get();
			material->setDiffuseMap(textureManager->getImage("container.png"));
			material->setEmissionMap(textureManager->getImage("matrix.jpg"));
			material->setSpecularMap(textureManager->getImage("container_s.png"));
			material->setShininess(32);
		}

		std::vector<std::unique_ptr<MeshGL>> meshes;
		meshes.push_back(move(mesh));

		auto model = std::make_unique<ModelGL>(move(meshes), std::vector<std::unique_ptr<Material>>());

		models.push_back(std::move(model));

		ModelGL* result = models.back().get();
		modelTable[CUBE_POSITION_NORMAL_TEX_HASH] = result;

		return result;
	}

	nex::ModelManagerGL* nex::ModelManagerGL::get()
	{
		static ModelManagerGL instance;
		return &instance;
	}

	void nex::ModelManagerGL::loadModels()
	{
		//TODO
		getPositionNormalTexCube();


		//AssimpModelLoader::loadModel("");
	}

	void nex::ModelManagerGL::release()
	{
		modelTable.clear();
		models.clear();
	}

	/*void ModelManagerGL::useInstances(ModelGL* source, mat4* modelMatrices, unsigned int amount)
	{
		ModelGL* model = static_cast<ModelGL*>(source);
		model->createInstanced(amount, modelMatrices);
	}*/