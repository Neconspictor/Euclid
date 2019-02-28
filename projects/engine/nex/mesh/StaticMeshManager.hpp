#pragma once
#include <vector>
#include <unordered_map>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <nex/material/PbrMaterialLoader.hpp>
#include <nex/material/BlinnPhongMaterialLoader.hpp>
#include <memory>


namespace nex
{
	enum class ShaderType;
	class FileSystem;

	/**
	 * A mesh manager provides a central access point for creating and receiving
	 * 3d meshes. All memory allocated by a mesh is held by the manager and no user
	 * memory management is needed. If the mesh manager isn't needed anymore, the
	 * allocated memory can be released by calling the manager's release method.
	 */
	class StaticMeshManager
	{
	public:

		static const unsigned int SKYBOX_MODEL_HASH;
		static const  unsigned int SPRITE_MODEL_HASH;

		StaticMeshManager();

		static std::unique_ptr<StaticMesh> createSphere(unsigned xSegments, unsigned ySegments, std::unique_ptr<Material> material);

		/**
		 * NOTE: Has to be initialized on first use
		 */
		static StaticMeshManager* get();

		StaticMesh* getSkyBox();

		/**
		 * Provides access to a mesh by its name.
		 * NOTE: If the specfied mesh cannot be found, a MeshNotFoundException is thrown.
		 */
		StaticMesh* getModel(const std::string& meshName, MaterialType type);

		/*
		 * Provides read acces to a cube that has position, normal and texture coordinates.
		 */
		StaticMesh* getPositionNormalTexCube();

		/*
		* \param xPos : The x position of the sprite model measured in screen space.
		* \param yPos : The y position of the sprite model measured in screen space.
		* \param widthWeight : specifies the width of the model as a percentage of the active viewport width.
		*		A value of 1.0f means full viewport width, 0.0f means no width analogously.
		* \param heightWeight : specifies the height of the model as a percentage of the active viewport height.
		*/
		StaticMesh* getSprite();


		/**
		 * Initializes the model manager.
		 * @param meshFileSystem Used to resolve mesh file paths
		 */
		void init(FileSystem* meshFileSystem);


		/**
		 * loads all meshes
		 */
		void loadModels();

		void release();

		//void useInstances(ModelGL* model, glm::mat4* modelMatrices, unsigned int amount);

	private:

		StaticMeshManager(const StaticMeshManager&) = delete;
		StaticMeshManager& operator=(const StaticMeshManager&) = delete;

		std::vector<std::unique_ptr<StaticMesh>> models;
		std::unordered_map<unsigned int, StaticMesh*> modelTable;
		MeshLoader assimpLoader;
		PbrMaterialLoader pbrMaterialLoader;
		BlinnPhongMaterialLoader blinnPhongMaterialLoader;
		FileSystem* mFileSystem;

		unsigned int CUBE_POSITION_NORMAL_TEX_HASH;
	};
}