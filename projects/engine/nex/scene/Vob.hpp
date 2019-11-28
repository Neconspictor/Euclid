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
	class Mesh;
	class MeshGroup;
	class MeshBatch;
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
		explicit Vob(Vob* parent, std::list<MeshBatch>* batches);

		virtual ~Vob();

		void addChild(Vob* child);
		std::list<MeshBatch>* getBatches();
		const std::list<MeshBatch>* getBatches() const;
		const AABB& getBoundingBox() const;
		std::list<Vob*>& getChildren();
		const std::list<Vob*>& getChildren() const;
		const glm::vec3& getPosition() const;
		const glm::quat& getRotation() const;

		Vob* getParent();
		const Vob* getParent() const;


		const glm::vec3& getScale() const;
		bool getSelectable() const;

		VobType getType() const;

		const glm::mat4& getWorldTrafo() const;
		const glm::mat4& getPrevWorldTrafo() const;

		bool isDeletable() const;

		void rotateGlobal(const glm::vec3& axisWorld, float angle);
		void rotateGlobal(const glm::vec3& eulerAngles);
		void rotateLocal(const glm::vec3& eulerAngles);

		void setBatches(std::list<MeshBatch>* batches);

		void setDeletable(bool deletable);

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
		virtual void updateTrafo(bool resetPrevWorldTrafo = false);

		void updateWorldTrafoHierarchy(bool resetPrevWorldTrafo = false);

		std::string mDebugName;

	protected:

		void recalculateBoundingBox();

		void updateWorldTrafo(bool resetPrevWorldTrafo);

		std::list<MeshBatch>* mBatches;
		std::list<Vob*> mChildren;
		Vob* mParent;

		glm::vec3 mPosition;
		glm::quat mRotation;
		glm::vec3 mScale;

		glm::mat4 mLocalTrafo;
		glm::mat4 mWorldTrafo;
		glm::mat4 mPrevWorldTrafo;

		bool mSelectable;
		bool mIsDeletable;
		AABB mBoundingBox;

		// Note: We use this meber field for optimization (avoids dynamic casts)
		VobType mType;
	};

	class MeshOwningVob : public Vob {
	public:

		MeshOwningVob(Vob* parent, std::unique_ptr<MeshGroup> group);

		void setMeshContainer(std::unique_ptr<MeshGroup> group);

		MeshGroup* getMesh() const;

		virtual ~MeshOwningVob();
	protected:
		std::unique_ptr<MeshGroup> mContainer;
	};

	class Billboard : public Vob, public FrameUpdateable {
	public:
		Billboard(Vob* parent, Vob* child);
		Billboard(Vob* parent, std::list<MeshBatch>* batches);
		virtual ~Billboard() = default;

		void frameUpdate(const Constants& constants) override;

		void updateTrafo(bool resetPrevWorldTrafo = false) override;
	};


	class RiggedVob : public Vob, public FrameUpdateable {
	public:

		RiggedVob(Vob* parent, std::list<MeshBatch>* batches);
		virtual ~RiggedVob();

		void frameUpdate(const Constants& constants) override;

		const std::vector<glm::mat4>& getBoneTrafos() const;
		
		void setActiveAnimation(const std::string& animationName);
		void setActiveAnimation(const BoneAnimation* animation);
		void setRepeatType(AnimationRepeatType type);

	protected:

		static const Mesh* findFirstLegalMesh(std::list<MeshBatch>* batches);

		void updateTime(float frameTime);

		const BoneAnimation* mActiveAnimation;
		const Rig* mRig;
		float mAnimationTime;
		AnimationRepeatType mRepeatType = AnimationRepeatType::LOOP;
		std::vector<glm::mat4> mBoneTrafos;
	};
}