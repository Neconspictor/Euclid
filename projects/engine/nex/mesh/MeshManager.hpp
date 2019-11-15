#pragma once
#include <vector>
#include <unordered_map>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <memory>
#include <nex/material/Material.hpp>


namespace nex
{
	enum class ShaderType;
	class FileSystem;
	class MeshAABB;
	class SphereMesh;

	/**
	 * A mesh manager provides a central access point for creating and receiving
	 * 3d meshes. All memory allocated by a mesh is held by the manager and no user
	 * memory management is needed. If the mesh manager isn't needed anymore, the
	 * allocated memory can be released by calling the manager's release method.
	 */
	class MeshManager
	{
	public:

		MeshManager();
		~MeshManager();

		static std::unique_ptr<MeshContainer> createSphere(unsigned xSegments, unsigned ySegments, std::unique_ptr<Material> material);


		/**
		 * NOTE: Has to be initialized on first use
		 */
		static MeshManager* get();



		MeshContainer* getSkyBox();

		/**
		 * Provides access to a mesh by its name.
		 * NOTE: If the specfied mesh cannot be found, a MeshNotFoundException is thrown.
		 * @param meshPath : The relative or absolute mesh path. The MeshManager's FileSystem
		 *						will be used to resolve the path.
		 * @param fileSystem : (Optional). If not null, this FileSystem will be used for resolving the
		 *						mesh path.
		 * @meshLoader : (Optional). If not null, the meshLoader will be used if the mesh isn't compiled yet.
		 * @materialLoader: Used to create materials from the mesh.
		 */
		MeshContainer* loadModel(const std::filesystem::path& meshPath,
			const nex::AbstractMaterialLoader& materialLoader,
			nex::AbstractMeshLoader* meshLoader = nullptr,
			const FileSystem* fileSystem = nullptr);

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
		MeshContainer* getPositionNormalTexCube();

		/*
		* \param xPos : The x position of the sprite model measured in screen space.
		* \param yPos : The y position of the sprite model measured in screen space.
		* \param widthWeight : specifies the width of the model as a percentage of the active viewport width.
		*		A value of 1.0f means full viewport width, 0.0f means no width analogously.
		* \param heightWeight : specifies the height of the model as a percentage of the active viewport height.
		*/
		MeshContainer* getSprite();

		MeshAABB* getUnitBoundingBoxLines();
		MeshAABB* getUnitBoundingBoxTriangles();
		SphereMesh* getUnitSphereTriangles();



		/**
		 * Initializes the mesh manager.
		 * @param meshFileSystem Used to resolve mesh file paths
		 */
		static void init(std::filesystem::path meshRootPath,
			std::string compiledSubFolder,
			std::string compiledFileExtension);


		/**
		 * loads all meshes
		 */
		void loadModels();

		static void release();

		//void useInstances(ModelGL* model, glm::mat4* modelMatrices, unsigned int amount);

	private:

		MeshManager(const MeshManager&) = delete;
		MeshManager& operator=(const MeshManager&) = delete;

		std::vector<std::unique_ptr<MeshContainer>> models;
		std::unordered_map<unsigned int, MeshContainer*> modelTable;
		std::unique_ptr<FileSystem> mFileSystem;
		std::unique_ptr<VertexArray> mFullscreenPlane;
		std::unique_ptr<VertexBuffer> mFullscreenPlaneData;
		std::unique_ptr<VertexArray> mFullscreenTriangle;
		std::unique_ptr<VertexBuffer> mFullscreenTriangleData;
		std::unique_ptr<nex::MeshAABB> mUnitBoundingBoxLines;
		std::unique_ptr<nex::MeshAABB> mUnitBoundingBoxTriangles;
		std::unique_ptr<nex::SphereMesh> mUnitSphereTriangles;
		static std::unique_ptr<MeshManager> mInstance;

		bool mInitialized;

		unsigned int CUBE_POSITION_NORMAL_TEX_HASH;
		unsigned int SKYBOX_MODEL_HASH;
		unsigned int SPRITE_MODEL_HASH;
	};
}
