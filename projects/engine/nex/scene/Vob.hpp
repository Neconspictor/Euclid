#pragma once
#include <memory>
#include <nex/math/BoundingBox.hpp>
#include <nex/common/FrameUpdateable.hpp>
#include <nex/anim/AnimationType.hpp>


#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/quaternion.hpp>

namespace nex
{
	class SceneNode;
	class Mesh;
	class MeshGroup;
	class Rig;
	class BoneAnimation;


	enum class VobType {
		Normal,
		Probe,
		Skinned
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

		MeshOwningVob(std::unique_ptr<MeshGroup> group);

		void setMeshContainer(std::unique_ptr<MeshGroup> group);

		MeshGroup* getMesh() const;

		virtual ~MeshOwningVob();
	protected:
		std::unique_ptr<MeshGroup> mContainer;
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

		static const Mesh* findFirstLegalMesh(SceneNode* node);

		void updateTime(float frameTime);

		const BoneAnimation* mActiveAnimation;
		const Rig* mRig;
		float mAnimationTime;
		AnimationRepeatType mRepeatType = AnimationRepeatType::LOOP;
		std::vector<glm::mat4> mBoneTrafos;
	};
}