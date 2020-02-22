#pragma once
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <filesystem>
#include <vector>

namespace nex
{
	/**
	 * A 3d scene imported from 3d application exchange file formats (e.g. *.obj, *.3ds, *.fbx, etc...).
	 */
	class ImportScene
	{
	public:
		ImportScene();
		~ImportScene();

		ImportScene(ImportScene&&) = default;
		ImportScene& operator=(ImportScene&&) = default;

		ImportScene(const ImportScene&) = delete;
		ImportScene& operator=(const ImportScene&) = delete;


		std::unordered_set<const aiNode*> collectBones() const;
		std::vector<const aiNode*> getRootBones(const std::unordered_set<const aiNode*>& bones) const;
		const aiNode* getFirstRootBone(bool assertUnique = true) const;

		/**
		 * Reads a 3d scene from file and processes it with default applied assimp flags.
		 * @throws std::runtime_error : If the scene couldn't be loaded.
		 */
		static ImportScene read(const std::filesystem::path& file, bool doMeshOptimizations);

		/**
		 * Provides the file path of the loaded scene. The path will be absolute.
		 */
		const std::filesystem::path& getFilePath()const;
		const aiScene* getAssimpScene()const;

		/**
		 * Searches a scene node by name
		 */
		const aiNode* getNode(const aiString& name) const;

		static glm::mat4 convert(const aiMatrix4x4& mat);
		static glm::vec3 convert(const aiVector3D& vec);
		static glm::quat convert(const aiQuaternion& quat);

		/**
		 * Checks if the scene contains bone animation data
		 */
		bool hasBoneAnimations() const;

		/**
		 * Checks if the scene contains vertices with assigned bones.
		 */
		bool hasBones() const;

		/**
		 * Checks, if the mesh data is valid.
		 */
		bool meshDataIsValid() const;


	private:

		class DebugSceneNode {
		public:
			std::vector<DebugSceneNode> children;
			const aiNode* node;

			static std::unique_ptr<DebugSceneNode> create(const aiScene* scene);
		};

		std::unique_ptr<Assimp::Importer> mImporter;
		const aiScene* mAssimpScene;
		std::unique_ptr<DebugSceneNode> mDebugSceneNodeRoot;
		std::filesystem::path mFile;
	};
}