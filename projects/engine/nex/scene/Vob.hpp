#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/common/FrameUpdateable.hpp>
#include <nex/anim/AnimationType.hpp>
#include <nex/renderer/RenderCommandFactory.hpp>
#include <nex/math/TrafoSpace.hpp>
#include <interface/buffers.h>


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

		void applyTrafoLocalToWorld(const glm::mat4& trafoLocalToWorld, const glm::vec3& origin = glm::vec3(0.0f));

		void collectRenderCommands(RenderCommandQueue& queue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer) override;

		void removeChild(Vob* child);

		std::vector<MeshBatch>* getBatches();
		const std::vector<MeshBatch>* getBatches() const;
		const AABB& getBoundingBoxWorld() const;
		const nex::AABB& getBoundingBoxLocal() const;
		std::vector<Vob*>& getChildren();
		const std::vector<Vob*>& getChildren() const;
		
		std::string& getName();
		const std::string& getName() const;
		std::string& getTypeName();
		const std::string& getTypeName() const;


		const glm::vec3& getPositionLocalToParent() const;
		const glm::vec3& getPositionLocalToWorld() const;

		Vob* getParent();
		const Vob* getParent() const;

		const PerObjectMaterialData& getPerObjectMaterialData() const;
		PerObjectMaterialData& getPerObjectMaterialData();

		unsigned getPerObjectMaterialDataID() const;

		const glm::quat& getRotationLocalToParent() const;
		glm::quat getRotationLocalToWorld() const;


		glm::vec3 getScaleLocalToWorld() const;
		const glm::vec3& getScaleLocalToParent() const;
		bool getSelectable() const;



		const glm::mat4& getTrafoMeshToLocal() const;
		const glm::mat4& getTrafoLocalToParent() const;
		const glm::mat4& getTrafoLocalToWorld() const;
		const glm::mat4& getTrafoMeshToWorld() const;
		const glm::mat4& getTrafoPrevMeshToWorld() const;

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

		virtual void setBatches(std::vector<MeshBatch>* batches);

		void setDeletable(bool deletable);

		void setRotationLocalToParent(const glm::vec3& eulerAngles);


		void setParent(Vob* parent);

		void setPerObjectMaterialData(const PerObjectMaterialData& data);
		void setPerObjectMaterialDataID(unsigned id);

		/**
		 * Sets the position of this vob.
		 */
		virtual void setPositionLocalToParent(const glm::vec3& position);
		void setPositionLocalToWorld(const glm::vec3& position);

		/**
		 * Sets the scale of this vob.
		 */
		void setScaleLocalToParent(const glm::vec3& scale);
		void setScaleLocalToWorld(const glm::vec3& scale, const glm::vec3& minOldScale = glm::vec3(0.0001f));

		void setSelectable(bool selectable);

		void setRotationLocalToParent(const glm::mat4& rotation);
		void setRotationLocalToParent(const glm::quat& rotation);
		void setRotationLocalToWorld(const glm::quat& rotation);


		void setTrafoLocalToParent(const glm::mat4& mat);
		void setTrafoMeshToLocal(const glm::mat4& mat);



		virtual void updateTrafo(bool resetPrevWorldTrafo = false, bool recalculateBoundingBox = true);

		virtual void updateWorldTrafoHierarchy(bool resetPrevWorldTrafo = false);

		virtual void recalculateBoundingBoxWorld();
		virtual void recalculateLocalBoundingBox();

	

		void updateWorldTrafo(bool resetPrevWorldTrafo);

		protected:

		std::vector<MeshBatch>* mBatches = nullptr;
		std::vector<Vob*> mChildren;
		Vob* mParent;

		std::string mName;
		std::string mTypeName;

		TrafoSpace mLocalToParentSpace;

		glm::mat4 mTrafoMeshToLocal;
		glm::mat4 mTrafoMeshToWorld;
		glm::mat4 mTrafoPrevMeshToWorld;
		//glm::mat4 mTrafoLocalToParent;
		glm::mat4 mTrafoLocalToWorld;

		bool mSelectable;
		bool mIsDeletable;

		AABB mBoundingBoxLocal;
		AABB mBoundingBoxWorld;

		bool mInheritParentScale;
		bool mNoUpdate = false;

		unsigned mPerObjectMaterialDataID;
		PerObjectMaterialData mPerObjectMaterialData;
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

		virtual void setBatches(std::vector<MeshBatch>* batches);

	protected:

		static const Mesh* findFirstLegalMesh(std::vector<MeshBatch>* batches);

		void updateTime(float frameTime);

		const BoneAnimation* mActiveAnimation;
		const Rig* mRig;
		float mAnimationTime;
		AnimationRepeatType mRepeatType = AnimationRepeatType::LOOP;
		std::vector<glm::mat4> mBoneTrafos;
	};
}