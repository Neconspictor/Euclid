#pragma once
#include <nex/anim/Rig.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <filesystem>


namespace nex {

	class ImportScene;
	class FileSystem;
	
	class AnimationManager {
	public:

		~AnimationManager();

		/**
		 * Adds a rig.
		 * @throws std::invalid_argument : if the manager contains a rig having the same id as the rig to be added.
		 */
		void add(std::unique_ptr<Rig> rig);

		/**
		 * Provides a rig by its id.
		 * If no suitable rig is found, null is returned.
		 */
		const Rig* getByID(unsigned id) const;

		/**
		 * Provides the rig the mesh container is associated with.
		 * @throws std::invalid_argument : if container has no mesh with an associated rig.
		 */
		const Rig* getRig(const MeshContainer& container) const;

		/**
		 * Provides access to the animation manager's FileSystem.
		 */
		const FileSystem* getFileSystem() const;

		/**
		 * Provides the rig manager.
		 */
		static AnimationManager* get();

		/**
		 * Inits the rig manager.
		 */
		static void init(
			const std::filesystem::path& animationRootPath,
			const std::string& compiledSubFolder,
			const std::string& compiledAnimationFileExtension,
			const std::string& compiledRigFileExtension);

		/**
		 * Loads a Rig from a scene.
		 */
		const Rig* load(const ImportScene& importScene);

	private:
		std::unordered_map<unsigned, std::unique_ptr<Rig>> mRigs;
		std::unique_ptr<FileSystem> mFileSystem;
	};
}