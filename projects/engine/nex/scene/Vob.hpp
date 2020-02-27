#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/common/FrameUpdateable.hpp>
#include <nex/anim/AnimationType.hpp>
#include <nex/renderer/RenderCommandFactory.hpp>
#include <nex/math/TrafoSpace.hpp>
#include <interface/buffers.h>
#include <nex/util/Memory.hpp>
#include <nex/anim/KeyFrameAnimation.hpp>

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

	class Vob : public nex::RenderCommandFactory, public FrameUpdateable
	{
	public:

		using MeshGroupPtr = flexible_ptr<MeshGroup>;
		using ChildPtr = flexible_ptr<Vob>;

		explicit Vob();

		virtual ~Vob();

		/**
		 * Note: The memory of the child isn't managed by this class!
		 */
		void addChild(ChildPtr child);

		void applyTrafoLocalToWorld(const glm::mat4& trafoLocalToWorld, const glm::vec3& origin = glm::vec3(0.0f));

		void collectRenderCommands(RenderCommandQueue& queue, bool doCulling, const RenderContext& renderContext) const override;


		std::unique_ptr<Vob> createBluePrintCopy() const;

		void finalizeMeshes();

		void frameUpdate(const RenderContext& constants) override;

		/**
		 * Removes a child by its pointer.
		 * @return: the child. Managed pointer is null, if no child was found to remove
		 */
		ChildPtr removeChild(Vob* child);

		/**
		 * Provides the blue-print vob used to create this vob.
		 * Result is nullptr, if no blue-print vob was used.
		 */
		const nex::Vob* getBluePrint() const;
		/**
		 * Provides the name SID of the blue-print vob.
		 * Note: The result is only valid, if this vob was created by a blue-print.
		 */
		unsigned getBluePrintNameSID() const;

		MeshGroup* getMeshGroup();
		const MeshGroup* getMeshGroup() const;

		const AABB& getBoundingBoxWorld() const;
		const nex::AABB& getBoundingBoxLocal() const;
		std::vector<ChildPtr>& getChildren();
		const std::vector<ChildPtr>& getChildren() const;
		
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
		bool usesPerObjectMaterialData() const;
		void usePerObjectMaterialData(bool val);

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
		bool isStatic() const;

		void rotateGlobal(const glm::vec3& axisWorld, float angle);
		void rotateGlobal(const glm::vec3& eulerAngles);
		void rotateLocal(const glm::vec3& eulerAngles);

		void setActiveKeyFrameAnimation(nex::Sid sid);
		const KeyFrameAnimation* getActiveKeyFrameAnimation() const;
		void addKeyFrameAnimations(std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>> anis);
		const std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>>& getKeyFrameAnimations() const;


		virtual void setMeshGroup(MeshGroupPtr meshGroup);

		void setDeletable(bool deletable);

		void setIsStatic(bool isStatic);

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
		
		std::unique_ptr<Vob> createBluePrintRecursive() const;

		virtual std::unique_ptr<Vob> createNew() const;

		MeshGroupPtr mMeshGroup;
		std::vector<ChildPtr> mChildren;
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
		bool mUsesPerObjectMaterialData;

		bool mIsStatic = false;

		// The blue print of this vob.
		// Can be nullptr
		const nex::Vob* mBluePrint = nullptr;
		unsigned mBluePrintNameSID = 0;
		nex::Sid mActiveKeyFrameAniSID = 0;
		std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>> mKeyFrameAnis;
	};


	class Billboard : public Vob {
	public:
		Billboard();
		virtual ~Billboard() = default;

		void frameUpdate(const RenderContext& constants) override;

	protected:
		std::unique_ptr<Vob> createNew() const override;
	};


	class RiggedVob : public Vob {
	public:

		RiggedVob();
		virtual ~RiggedVob();

		void collectRenderCommands(RenderCommandQueue& queue, bool doCulling, const RenderContext& renderContext) const override;

		void frameUpdate(const RenderContext& constants) override;

		/**
		 * Note: Result can be null, if no animation is active.
		 */
		const BoneAnimation* getActiveAnimation() const;

		const std::vector<glm::mat4>& getBoneTrafos() const;
		const Rig* getRig() const;

		void pauseAnimation(bool pause);
		bool isAnimationPaused() const;
		
		void setActiveAnimation(const std::string& animationName);
		void setActiveAnimation(const BoneAnimation* animation);
		void setRepeatType(AnimationRepeatType type);

		void setMeshGroup(MeshGroupPtr meshGroup) override;


		void recalculateLocalBoundingBox() override;

	protected:

		std::unique_ptr<Vob> createNew() const override;

		static const Mesh* findFirstLegalMesh(std::vector<MeshBatch>* batches);

		void updateTime(float frameTime);

		const BoneAnimation* mActiveAnimation = nullptr;
		const Rig* mRig = nullptr;
		float mAnimationTime;
		AnimationRepeatType mRepeatType = AnimationRepeatType::LOOP;
		std::vector<glm::mat4> mBoneTrafos;
		bool mIsPaused = false;
	};
}