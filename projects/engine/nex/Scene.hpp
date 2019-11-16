#pragma once
#include <unordered_set>
#include <nex/drawing/MeshDrawer.hpp>
#include <vector>
#include <memory>
#include <nex/util/Iterator.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/common/FrameUpdateable.hpp>
#include <nex/anim/AnimationType.hpp>
//#include <nex/anim/BoneAnimation.hpp>
//#include <nex/anim/Rig.hpp>


#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/quaternion.hpp>
#include <nex/common/Concurrent.hpp>

namespace nex
{
	class Mesh;
	class SkinnedMesh;
	class Material;
	class ProbeVob;
	class MeshContainer;
	class RenderCommandQueue;
	class Rig;
	class BoneAnimation;


	enum class VobType {
		Normal,
		Probe
	};


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
		Mesh* getMesh() const;
		Material* getMaterial() const;
		const nex::AABB& getMeshBoundingBoxWorld() const;
		SceneNode* getParent();

		
		const glm::mat4& getWorldTrafo() const;
		const glm::mat4& getPrevWorldTrafo() const;
		
		
		void setMesh(Mesh* mesh);
		void setMaterial(Material* material);
		void setParent(SceneNode* parent);

		
		void setLocalTrafo(const glm::mat4& mat);

		void updateChildrenWorldTrafos(bool resetPrevWorldTrafo = false);
		void updateWorldTrafoHierarchy(bool resetPrevWorldTrafo = false);
		

		std::string mDebugName;

	private:

		void updateWorldTrafo(bool resetPrevWorldTrafo);

		Children mChildren;
		Mesh* mMesh;

		Material* mMaterial;
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

		void collectRenderCommands(RenderCommandQueue& commandQueue, bool doCulling) const;

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


	class Vob
	{
	public:
		explicit Vob(SceneNode* meshRootNode);

		virtual ~Vob();

		const SceneNode* getMeshRootNode() const;
		SceneNode* getMeshRootNode();

		const AABB& getBoundingBox() const;
		const glm::vec3& getPosition() const;
		const glm::quat& getRotation() const;
		const glm::vec3& getScale() const;
		bool getSelectable() const;

		VobType getType() const;

		bool isDeletable() const;

		void rotateGlobal(const glm::vec3& axisWorld, float angle);
		void rotateGlobal(const glm::vec3& eulerAngles);
		void rotateLocal(const glm::vec3& eulerAngles);

		void setDeletable(bool deletable);

		/**
		 * Sets the root mesh node for this vob.
		 * Note: Takes ownership of the node! Deletes the old mesh root node (if existing)
		 */
		void setMeshRootNode(SceneNode* node);

		void setOrientation(const glm::vec3& eulerAngles);

		/**
		 * Sets the position of this vob.
		 */
		virtual void setPosition(const glm::vec3& position);

		/**
		 * Sets the scale of this vob.
		 */
		void setScale(const glm::vec3& scale);

		void setSelectable(bool selectable);
		void setRotation(const glm::mat4& rotation);
		void setRotation(const glm::quat& rotation);

		/**
		 * Sets the visual transformation of this vob
		 * based on a matrix.
		 */
		void setTrafo(const glm::mat4& mat);


		/**
		 * Calculates the transformation matrix of this vob
		 * based on its position, scale and rotation.
		 */
		void updateTrafo(bool resetPrevWorldTrafo = false);

		std::string mDebugName;

	protected:

		void recalculateBoundingBox();

		SceneNode* mMeshRootNode;
		glm::vec3 mPosition;
		glm::quat mRotation;
		glm::vec3 mScale;
		bool mSelectable;
		bool mIsDeletable;
		AABB mBoundingBox;

		// Note: We use this meber field for optimization (avoids dynamic casts)
		VobType mType;
	};

	class MeshOwningVob : public Vob {
	public:

		MeshOwningVob(std::unique_ptr<MeshContainer> container);

		void setMeshContainer(std::unique_ptr<MeshContainer> container);

		MeshContainer* getMesh() const;

		virtual ~MeshOwningVob();
	protected:
		std::unique_ptr<MeshContainer> mContainer;
	};


	class RiggedVob : public Vob, public FrameUpdateable {
	public:

		RiggedVob(SceneNode* meshRootNode);
		virtual ~RiggedVob();

		void frameUpdate(float frameTime) override;

		const std::vector<glm::mat4>& getBoneTrafos() const;
		
		void setActiveAnimation(const std::string& animationName);
		void setActiveAnimation(const BoneAnimation* animation);
		void setRepeatType(AnimationRepeatType type);

	protected:

		static Mesh* findFirstLegalMesh(SceneNode* node);

		void updateTime(float frameTime);

		const BoneAnimation* mActiveAnimation;
		const Rig* mRig;
		float mAnimationTime;
		AnimationRepeatType mRepeatType = AnimationRepeatType::LOOP;
		std::vector<glm::mat4> mBoneTrafos;
	};
}