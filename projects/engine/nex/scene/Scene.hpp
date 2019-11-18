#pragma once
#include <unordered_set>
#include <nex/drawing/MeshDrawer.hpp>
#include <vector>
#include <memory>
#include <nex/util/Iterator.hpp>
#include <nex/math/BoundingBox.hpp>


#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/quaternion.hpp>
#include <nex/common/Concurrent.hpp>

namespace nex
{
	class MeshBatch;
	class ProbeVob;
	class MeshGroup;
	class RenderCommandQueue;


	class SceneNode
	{
	public:

		using Children = std::vector<SceneNode*>;

		SceneNode() noexcept;
		~SceneNode();

		/**
		 * Note: This object takes ownership of the child node!
		 */
		void addChild(SceneNode* node);
		void clear();
		const Children& getChildren() const;
		MeshBatch* getBatch() const;
		const nex::AABB& getMeshBoundingBoxWorld() const;
		SceneNode* getParent();
		
		const glm::mat4& getWorldTrafo() const;
		const glm::mat4& getPrevWorldTrafo() const;
		
		void setBatch(MeshBatch* mesh);
		void setParent(SceneNode* parent);

		void setLocalTrafo(const glm::mat4& mat);

		void updateChildrenWorldTrafos(bool resetPrevWorldTrafo = false);
		void updateWorldTrafoHierarchy(bool resetPrevWorldTrafo = false);
		

		std::string mDebugName;

	private:

		void updateWorldTrafo(bool resetPrevWorldTrafo);

		Children mChildren;
		MeshBatch* mBatch;

		SceneNode* mParent;
		glm::mat4 mLocalTrafo;
		glm::mat4 mWorldTrafo;
		glm::mat4 mPrevWorldTrafo;
		nex::AABB mBoundingBox;
	};

	/**
	 * A scene manages the creation and lifetime of scene nodes.
	 * A scene is a list of trees;
	 */
	class Scene
	{
	public:

		using VobRange = std::unordered_set<Vob*>;
		using ProbeRange = std::unordered_set<ProbeVob*>;

		/**
		 * Creates a new scene object.
		 */
		Scene();

		UniqueLock acquireLock() const;

		void addActiveVobUnsafe(Vob* vob);
		void removeActiveVobUnsafe(Vob* vob);
		bool deleteVobUnsafe(Vob* vob);


		Vob* addVobUnsafe(std::unique_ptr<Vob> vob, bool setActive = true);
		Vob* createVobUnsafe(SceneNode* meshRootNode, bool setActive = true);

		/**
		 * Provides all vobs that are currently active.
		 */
		const VobRange& getActiveVobsUnsafe() const;

		/**
		 * Provides all probe vobs that are currently active.
		 */
		const ProbeRange& getActiveProbeVobsUnsafe() const;

		/**
		 * Provides all vobs of this scene.
		 */
		const std::vector<std::unique_ptr<Vob>>& getVobsUnsafe() const;
		std::vector<std::unique_ptr<Vob>>& getVobsUnsafe();

		bool hasChangedUnsafe() const;


		/**
		 * Checks if a vob is active.
		 */
		bool isActive(Vob* vob) const;

		/**
		 * Deletes all nodes except the root node.
		 */
		void clearUnsafe();
		void updateWorldTrafoHierarchyUnsafe(bool resetPrevWorldTrafo);

		void calcSceneBoundingBoxUnsafe();
		const AABB& getSceneBoundingBox() const;

		void collectRenderCommands(RenderCommandQueue& commandQueue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer) const;

		void setHasChangedUnsafe(bool changed);

	private:
		std::unordered_set<Vob*> mActiveVobs;
		std::unordered_set<ProbeVob*> mActiveProbeVobs;
		std::vector<std::unique_ptr<Vob>> mVobStore;
		mutable std::mutex mMutex;
		AABB mBoundingBox;
		bool mHasChanged;
	};


	class DefaultHierarchyFactory;
	class HierarchyFactory;
	using ManagedNodeHierarchyFactory = std::unique_ptr<HierarchyFactory>;

	class HierarchyFactory {
	public:
		virtual ~HierarchyFactory() = default;

		/**
		 * Creates a SceneNode* hierarchy that can be deleted via the delete operator.
		 */
		virtual SceneNode* createNodeHierarchy() = 0;
	};

	class DefaultHierarchyFactory : public HierarchyFactory {
	public:
		DefaultHierarchyFactory(SceneNode* root) : mRoot(root) {}

		SceneNode* createNodeHierarchy() override {
			// assure that we provide it only one time 
			// (result has to be deletable!)
			auto* result = mRoot;
			if (mRoot) mRoot = nullptr;
			return mRoot;
		}

	private:
		SceneNode* mRoot;
	};
}