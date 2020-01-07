#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/common/FrameUpdateable.hpp>
#include <nex/anim/AnimationType.hpp>
#include <nex/renderer/RenderCommandFactory.hpp>


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

	class Vob : public nex::RenderCommandFactory
	{
	public:
		explicit Vob(Vob* parent = nullptr);

		virtual ~Vob();

		/**
		 * Note: The memory of the child isn't managed by this class!
		 */
		void addChild(Vob* child);

		void applyWorldTransformation(const glm::mat4& trafoWorldSpace, const glm::vec3& origin = glm::vec3(0.0f));

		void collectRenderCommands(RenderCommandQueue& queue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer) override;

		void removeChild(Vob* child);

		std::list<MeshBatch>* getBatches();
		const std::list<MeshBatch>* getBatches() const;
		const AABB& getBoundingBox() const;
		const nex::AABB& getLocalBoundingBox() const;
		std::vector<Vob*>& getChildren();
		const std::vector<Vob*>& getChildren() const;
		
		std::string& getName();
		const std::string& getName() const;
		std::string& getTypeName();
		const std::string& getTypeName() const;


		const glm::vec3& getPositionLocal() const;
		const glm::vec3& getPositionWorld() const;

		Vob* getParent();
		const Vob* getParent() const;

		const glm::quat& getRotationLocal() const;
		glm::quat getRotationWorld() const;


		glm::vec3 getScaleWorld() const;
		const glm::vec3& getScaleLocal() const;
		bool getSelectable() const;

		const glm::mat4& getTrafoLocal() const;
		const glm::mat4& getTrafoWorld() const;
		const glm::mat4& getTrafoPrevWorld() const;

		void inheritParentScale(bool inherit);

		/**
		 * Checks if the given vob is a (direct or indirect) child of this vob.
		 */
		bool hasChild(const Vob* vob) const;

		bool isDeletable() const;
		bool isParentScaleInherited() const;
		bool isRoot() const;

		void rotateGlobal(const glm::vec3& axisWorld, float angle);
		void rotateGlobal(const glm::vec3& eulerAngles);
		void rotateLocal(const glm::vec3& eulerAngles);

		virtual void setBatches(std::list<MeshBatch>* batches);

		void setDeletable(bool deletable);

		void setOrientationLocal(const glm::vec3& eulerAngles);


		void setParent(Vob* parent);

		/**
		 * Sets the position of this vob.
		 */
		virtual void setPositionLocal(const glm::vec3& position);
		void setPositionWorld(const glm::vec3& position);

		/**
		 * Sets the scale of this vob.
		 */
		void setScaleLocal(const glm::vec3& scale);
		void setScaleWorld(const glm::vec3& scale);

		void setSelectable(bool selectable);
		void setRotationLocal(const glm::mat4& rotation);
		void setRotationLocal(const glm::quat& rotation);
		void setRotationWorld(const glm::quat& rotation);

		/**
		 * Sets the visual transformation of this vob
		 * based on a matrix.
		 */
		void setTrafoLocal(const glm::mat4& mat);


		/**
		 * Calculates the transformation matrix of this vob
		 * based on its position, scale and rotation.
		 */
		virtual void updateTrafo(bool resetPrevWorldTrafo = false, bool recalculateBoundingBox = true);

		virtual void updateWorldTrafoHierarchy(bool resetPrevWorldTrafo = false);

		virtual void recalculateBoundingBoxWorld();
		virtual void recalculateLocalBoundingBox();

	protected:

		void updateWorldTrafo(bool resetPrevWorldTrafo);

		std::list<MeshBatch>* mBatches;
		std::vector<Vob*> mChildren;
		Vob* mParent;

		std::string mName;
		std::string mTypeName;

		glm::vec3 mPosition;
		glm::quat mRotation;
		glm::vec3 mScale;

		glm::vec3 mPositionStacked;
		glm::quat mRotationStacked;
		glm::vec3 mScaleStacked;

		glm::mat4 mLocalTrafo;
		glm::mat4 mWorldTrafo;
		glm::mat4 mPrevWorldTrafo;

		bool mSelectable;
		bool mIsDeletable;

		AABB mBoundingBoxLocal;
		AABB mBoundingBoxWorld;

		bool mInheritParentScale;
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
		Billboard(Vob* parent);
		virtual ~Billboard() = default;

		void frameUpdate(const RenderContext& constants) override;
	};


	class RiggedVob : public Vob, public FrameUpdateable {
	public:

		RiggedVob(Vob* parent);
		virtual ~RiggedVob();

		void collectRenderCommands(RenderCommandQueue& queue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer) override;

		void frameUpdate(const RenderContext& constants) override;

		const std::vector<glm::mat4>& getBoneTrafos() const;
		const Rig* getRig() const;
		
		void setActiveAnimation(const std::string& animationName);
		void setActiveAnimation(const BoneAnimation* animation);
		void setRepeatType(AnimationRepeatType type);

		virtual void setBatches(std::list<MeshBatch>* batches);

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