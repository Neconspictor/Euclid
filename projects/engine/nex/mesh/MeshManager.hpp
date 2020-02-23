#pragma once
#include <vector>
#include <unordered_map>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <memory>
#include <nex/material/Material.hpp>
#include <nex/scene/Vob.hpp>


namespace nex
{
	enum class ShaderType;
	class FileSystem;
	class MeshAABB;
	class SphereMesh;
	class Vob;



	struct VobHierarchy {
		std::vector<std::unique_ptr<Vob>> vobs;
		Vob* root = nullptr;
	};


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

		static std::unique_ptr<MeshGroup> createSphere(unsigned xSegments, unsigned ySegments, std::unique_ptr<Material> material);


		/**
		 * NOTE: Has to be initialized on first use
		 */
		static MeshManager* get();

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
		std::unique_ptr<nex::MeshGroup> loadModel(const std::filesystem::path& meshPath,
			const nex::AbstractMaterialLoader& materialLoader,
			float rescale = 1.0f,
			bool forceLoad = false,
			nex::AbstractMeshLoader* meshLoader = nullptr,
			const FileSystem* fileSystem = nullptr);


		VobHierarchy loadVobHierarchy(const std::filesystem::path& meshPath,
			const nex::AbstractMaterialLoader& materialLoader,
			float rescale = 1.0f,
			bool forceLoad = false,
			const FileSystem* fileSystem = nullptr);


		const FileSystem& getFileSystem() const;

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

		VertexBuffer* getNDCFullscreenTriangleVB();

		MeshAABB* getUnitBoundingBoxLines();
		MeshAABB* getUnitBoundingBoxTriangles();
		const Mesh* getUnitPlane() const;
		SphereMesh* getUnitSphereTriangles();



		/**
		 * Initializes the mesh manager.
		 * @param meshFileSystem Used to resolve mesh file paths
		 */
		static void init(const std::filesystem::path& meshRootPath,
			const std::string& compiledSubFolder,
			const std::string& compiledFileExtension);


		static void release();

		//void useInstances(ModelGL* model, glm::mat4* modelMatrices, unsigned int amount);

	private:

		MeshManager(const MeshManager&) = delete;
		MeshManager& operator=(const MeshManager&) = delete;

		std::filesystem::path constructCompiledPath(const std::filesystem::path& absolutePath, 
			const FileSystem* filesystem, 
			float rescale, 
			const char* extension = nullptr);

		std::unique_ptr<Vob> createVob(const VobBaseStore& store, const AbstractMaterialLoader& materialLoader) const;
		bool checkIsSkinned(const VobBaseStore& store, std::string& rigIDOut) const;

		std::unique_ptr<FileSystem> mFileSystem;
		std::unique_ptr<VertexArray> mFullscreenPlane;
		std::unique_ptr<VertexBuffer> mFullscreenPlaneData;
		std::unique_ptr<VertexArray> mFullscreenTriangle;
		std::unique_ptr<VertexBuffer> mFullscreenTriangleData;
		std::unique_ptr<nex::MeshAABB> mUnitBoundingBoxLines;
		std::unique_ptr<nex::MeshAABB> mUnitBoundingBoxTriangles;
		std::unique_ptr<nex::SphereMesh> mUnitSphereTriangles;
		std::unique_ptr<nex::Mesh> mUnitPlane;
		static std::unique_ptr<MeshManager> mInstance;

		bool mInitialized;
	};
}
