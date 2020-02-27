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
		 * Loads bone animations from a file.
		 */
		std::vector<const BoneAnimation*> loadBoneAnimations(const std::filesystem::path& filePath);

		/**
		 * Loads keyframe animations (bone animation not included!) from a file.
		 */
		std::vector<std::unique_ptr<KeyFrameAnimation>> loadKeyFrameAnimations(const std::filesystem::path& filePath);

		/**
		 * Provides a bone animation by its name SID.
		 */
		const BoneAnimation* getBoneAnimation(unsigned sid);

		/**
		 * Provies the list of loaded bone animations for a specific rig.
		 * @throws std::runtime_error : if rig parameter isn't a rig registered on the animation manager.
		 */
		const std::set<const BoneAnimation*>& getBoneAnimationsByRig(const Rig* rig);

		/**
		 * Provides the rig the mesh container is associated with.
		 * @throws std::invalid_argument : if container has no mesh with an associated rig.
		 */
		const Rig* getRig(const MeshGroup& container);

		/**
		 * Provides the rig manager.
		 */
		static AnimationManager* get();

		/**
		 * Inits the rig manager.
		 */
		static void init(
			const std::filesystem::path& resourceRootPath,
			const std::string& compiledResourcePath,
			const std::string& compiledAnimationFileExtension,
			const std::string& compiledRiggedMeshFileExtension,
			const std::string& compiledRigFileExtension,
			const std::string& metaFileExtension);

		/**
		 * Loads a Rig from a scene.
		 */
		const Rig* load(const ImportScene& importScene);

		/**
		 * Loads a rig by its name or provides a chached value. 
		 * If the rig couldn't be found, a ResourceLoadException will be thrown.
		 * @throws ResourceLoadException : If the rig couldn't be loaded.
		 */
		const Rig* load(const std::string& rigID);

		const Rig* load(const ImportScene& importScene, const aiNode* root);

	private:

		const Rig* loadRigFromCompiled(const std::string& rigID);

		/**
		 * Loads bone animations from a file.
		 */
		std::unique_ptr<BoneAnimation> loadSingleBoneAnimation(const aiAnimation* aiBoneAni, const ImportScene& importScene);

		/**
		 * Loads a single keyframe animation from a file.
		 */
		std::unique_ptr<KeyFrameAnimation> loadSingleKeyFrameAnimation(const aiAnimation* aiKeyFrameAni, const ImportScene& importScene);

		static std::string generateUniqueKeyFrameAniName(const aiAnimation* aiKeyFrameAni, const ImportScene& importScene);

		static unsigned getKeyFrameAniIndex(const aiAnimation* aiKeyFrameAni, const aiScene* scene);

		std::unordered_map<unsigned, std::unique_ptr<Rig>> mRigs;
		std::unordered_map<unsigned, std::unique_ptr<BoneAnimation>> mBoneAnimations;
		std::unordered_map<const Rig*, std::set<const BoneAnimation*>> mRigToBoneAnimations;
		std::unordered_map<unsigned, const BoneAnimation*> mSidToBoneAnimation;
		
		std::unique_ptr<FileSystem> mAnimationFileSystem;
		std::unique_ptr<FileSystem> mRigFileSystem;
		std::string mMetaExt;
	};
}