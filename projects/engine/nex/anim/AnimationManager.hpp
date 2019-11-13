#pragma once
#include <nex/anim/Rig.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <filesystem>
#include <nex/anim/BoneAnimation.hpp>


namespace nex {

	class ImportScene;
	class FileSystem;
	
	/**
	 * Management class for animations and rigs.
	 */
	class AnimationManager {
	public:

		~AnimationManager();

		/**
		 * Adds a rig.
		 * @throws std::invalid_argument : if the manager contains a rig having the same id as the rig to be added.
		 */
		void add(std::unique_ptr<Rig> rig);

		/**
		 * Provides a rig by its sid.
		 * If no suitable rig is found, null is returned.
		 */
		const Rig* getBySID(unsigned sid) const;

		/**
		 * Loads a bone animation by its name.
		 */
		const BoneAnimation* loadBoneAnimation(const std::string& name);

		/**
		 * Provides a bone animation by its SID.
		 */
		const BoneAnimation* getBoneAnimation(unsigned sid);

		/**
		 * Provides the rig the mesh container is associated with.
		 * @throws std::invalid_argument : if container has no mesh with an associated rig.
		 */
		const Rig* getRig(const MeshContainer& container);

		/**
		 * Provides access to the animation manager's FileSystem.
		 */
		const FileSystem* getRiggedMeshFileSystem() const;

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
			const std::string& compiledRiggedMeshFileExtension,
			const std::string& compiledRigFileExtension);

		/**
		 * Loads a Rig from a scene.
		 */
		const Rig* load(const ImportScene& importScene);

		/**
		 * Loads the rig id from an animation or a rigged mesh.
		 */
		std::string loadRigID(const ImportScene& importScene);



	private:

		const Rig* loadRigFromCompiled(const std::string& rigID);

		std::unordered_map<unsigned, std::unique_ptr<Rig>> mRigs;
		std::vector<std::unique_ptr<BoneAnimation>> mBoneAnimations;
		std::unordered_map<const Rig*, std::vector<const BoneAnimation*>> mRigToBoneAnimations;
		std::unordered_map<unsigned, const BoneAnimation*> mSidToBoneAnimation;
		
		std::unique_ptr<FileSystem> mAnimationFileSystem;
		std::unique_ptr<FileSystem> mRiggedMeshFileSystem;
		std::unique_ptr<FileSystem> mRigFileSystem;
	};
}