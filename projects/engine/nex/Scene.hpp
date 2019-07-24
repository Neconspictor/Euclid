#pragma once
#include <unordered_set>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <vector>
#include <memory>
#include <nex/util/Iterator.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/quaternion.hpp>
#include <nex/common/Concurrent.hpp>

namespace nex
{
	class Mesh;
	class Material;
	class ProbeVob;


	enum class VobType {
		Normal,
		Probe
	};


	class SceneNode
	{
	public:

		using Children = std::vector<SceneNode*>;

		SceneNode();

		void addChild(SceneNode* node);
		void clear();
		const Children& getChildren() const;
		Mesh* getMesh() const;
		Material* getMaterial() const;
		const nex::AABB& getMeshBoundingBoxWorld() const;
		SceneNode* getParent();

		
		const glm::mat4& getWorldTrafo() const;
		const glm::mat4& getPrevWorldTrafo() const;
		void removeChild(SceneNode* node);
		
		
		void setMesh(Mesh* mesh);
		void setMaterial(Material* material);
		void setParent(SceneNode* parent);

		
		void setLocalTrafo(const glm::mat4& mat);

		void updateChildrenWorldTrafos(bool resetPrevWorldTrafo = false);
		void updateWorldTrafoHierarchy(bool resetPrevWorldTrafo = false);
		

		std::string mDebugName;

	private:

		void updateWorldTrafo(bool resetPrevWorldTrafo);

		std::vector<SceneNode*> mChildren;
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

		/**
		 * Creates a new node.
		 */
		SceneNode* createNodeUnsafe(SceneNode* parent = nullptr);

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

		/**
		 * Deletes all nodes except the root node.
		 */
		void clearUnsafe();
		void updateWorldTrafoHierarchyUnsafe(bool resetPrevWorldTrafo);

	private:
		std::unordered_set<Vob*> mActiveVobs;
		std::unordered_set<ProbeVob*> mActiveProbeVobs;
		std::vector<std::unique_ptr<SceneNode>> mNodes;
		std::vector<std::unique_ptr<Vob>> mVobStore;
		mutable std::mutex mMutex;
	};


	class Vob
	{
	public:
		explicit Vob(SceneNode* meshRootNode);

		virtual ~Vob() = default;

		const SceneNode* getMeshRootNode() const;
		SceneNode* getMeshRootNode();

		const AABB& getBoundingBox() const;
		const glm::vec3& getPosition() const;
		const glm::quat& getRotation() const;
		const glm::vec3& getScale() const;
		bool getSelectable() const;

		VobType getType() const;

		void rotateGlobal(const glm::vec3& axisWorld, float angle);
		void rotateGlobal(const glm::vec3& eulerAngles);
		void rotateLocal(const glm::vec3& eulerAngles);

		/**
		 * Sets the root mesh node for this vob.
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
		AABB mBoundingBox;

		// Note: We use this meber field for optimzation (avoids dynamic casts)
		VobType mType;
	};
}