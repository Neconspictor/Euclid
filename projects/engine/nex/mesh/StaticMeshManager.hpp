#pragma once
#include <vector>
#include <unordered_map>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <nex/material/PbrMaterialLoader.hpp>
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

		StaticMeshManager();

		static std::unique_ptr<StaticMeshContainer> createSphere(unsigned xSegments, unsigned ySegments, std::unique_ptr<Material> material);

		/**
		 * NOTE: Has to be initialized on first use
		 */
		static StaticMeshManager* get();



		StaticMeshContainer* getSkyBox();

		/**
		 * Provides access to a mesh by its name.
		 * NOTE: If the specfied mesh cannot be found, a MeshNotFoundException is thrown.
		 */
		StaticMeshContainer* getModel(const std::string& meshName, MaterialType type);

		/**
		 * Provides a vertex array holding four vertices forming a fullscreen plane.
		 * To render the plane, no index buffer is needed. It is sufficient to call
		 * RenderBackend::drawArrays(Topology::TRIANGLES_STRIP, 0, 4) after binding the returned vertex array.
		 * 
		 * TODO: Until now only opengl NDC is supported! Generalize it to support also DirectX NDC!
		 */
		VertexArray* getNDCFullscreenPlane();

		/**
		 * Provides a vertex array holding three vertices forming a fullscreen triangle.
		 * To render the triangle, no index buffer is needed. It is sufficient to call
		 * RenderBackend::drawArrays(Topology::TRIANGLES, 0, 3) after binding the returned vertex array.
		 *
		 * TODO: Until now only opengl NDC is supported! Generalize it to support also DirectX NDC!
		 */
		VertexArray* getNDCFullscreenTriangle();


		/*
		 * Provides read acces to a cube that has position, normal and texture coordinates.
		 */
		StaticMeshContainer* getPositionNormalTexCube();

		/*
		* \param xPos : The x position of the sprite model measured in screen space.
		* \param yPos : The y position of the sprite model measured in screen space.
		* \param widthWeight : specifies the width of the model as a percentage of the active viewport width.
		*		A value of 1.0f means full viewport width, 0.0f means no width analogously.
		* \param heightWeight : specifies the height of the model as a percentage of the active viewport height.
		*/
		StaticMeshContainer* getSprite();


		/**
		 * Initializes the model manager.
		 * @param meshFileSystem Used to resolve mesh file paths
		 */
		void init(FileSystem* meshFileSystem, std::unique_ptr<PbrMaterialLoader> pbrMaterialLoader);


		/**
		 * loads all meshes
		 */
		void loadModels();

		void release();

		void setPbrMaterialLoader(std::unique_ptr<PbrMaterialLoader> pbrMaterialLoader);

		//void useInstances(ModelGL* model, glm::mat4* modelMatrices, unsigned int amount);

	private:

		StaticMeshManager(const StaticMeshManager&) = delete;
		StaticMeshManager& operator=(const StaticMeshManager&) = delete;

		std::vector<std::unique_ptr<StaticMeshContainer>> models;
		std::unordered_map<unsigned int, StaticMeshContainer*> modelTable;
		MeshLoader assimpLoader;
		std::unique_ptr<PbrMaterialLoader> mPbrMaterialLoader;
		std::unique_ptr<DefaultMaterialLoader> mDefaultMaterialLoader;
		FileSystem* mFileSystem;
		std::unique_ptr<VertexArray> mFullscreenPlane;
		std::unique_ptr<VertexBuffer> mFullscreenPlaneData;
		std::unique_ptr<VertexArray> mFullscreenTriangle;
		std::unique_ptr<VertexBuffer> mFullscreenTriangleData;
		bool mInitialized;

		unsigned int CUBE_POSITION_NORMAL_TEX_HASH;
		unsigned int SKYBOX_MODEL_HASH;
		unsigned int SPRITE_MODEL_HASH;
	};
}