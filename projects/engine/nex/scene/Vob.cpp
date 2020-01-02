#include <nex/scene/Vob.hpp>
#include <nex/scene/Scene.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/math/Math.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/camera/Camera.hpp>

namespace nex
{
	Vob::Vob(Vob* parent) : 
		RenderCommandFactory(),
		mSelectable(true), mIsDeletable(true),
		mParent(parent), mBatches(nullptr),
		mPosition(0.0f),
		mScale(1.0f),
		mName("Normal vob"),
		mTypeName("Normal vob"),
		mInheritParentScale(true)
	{
		if (mParent) mParent->addChild(this);
	}

	Vob::~Vob() = default;

	void Vob::addChild(Vob* child)
	{
		mChildren.push_back(child);

		child->setParent(this);
	}

	void Vob::collectRenderCommands(RenderCommandQueue& queue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer)
	{
		if (!mBatches) return;

		RenderCommand command;

		for (const auto& batch : *mBatches) {
			command.batch = &batch;
			command.worldTrafo = &mWorldTrafo;
			command.prevWorldTrafo = &mPrevWorldTrafo;
			command.boundingBox = &mBoundingBoxWorld;

			command.isBoneAnimated = false;
			command.bones = nullptr;
			command.boneBuffer = nullptr;

			queue.push(command, doCulling);
		}
	}

	void Vob::removeChild(Vob* child)
	{
		mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), child), mChildren.end());
		child->setParent(nullptr);
	}

	std::list<MeshBatch>* Vob::getBatches()
	{
		return mBatches;
	}

	const std::list<MeshBatch>* Vob::getBatches() const
	{
		return mBatches;
	}

	const AABB& Vob::getBoundingBox() const
	{
		return mBoundingBoxWorld;
	}

	const nex::AABB& Vob::getLocalBoundingBox() const
	{
		return mBoundingBoxLocal;
	}

	std::vector<Vob*>& Vob::getChildren()
	{
		return mChildren;
	}

	const std::vector<Vob*>& Vob::getChildren() const
	{
		return mChildren;
	}

	std::string& Vob::getName()
	{
		return mName;
	}

	const std::string& Vob::getName() const
	{
		return mName;
	}

	std::string& Vob::getTypeName()
	{
		return mTypeName;
	}

	const std::string& Vob::getTypeName() const
	{
		return mTypeName;
	}

	const glm::vec3& Vob::getPositionLocal() const
	{
		return mPosition;
	}

	const glm::vec3& Vob::getPositionWorld() const
	{
		return reinterpret_cast<const glm::vec3&>(mWorldTrafo[3]);
	}

	const glm::quat& Vob::getRotationLocal() const
	{
		return mRotation;
	}

	Vob* Vob::getParent()
	{
		return mParent;
	}

	const Vob* Vob::getParent() const
	{
		return mParent;
	}

	const glm::vec3& Vob::getScaleLocal() const
	{
		return mScale;
	}

	bool Vob::getSelectable() const
	{
		return mSelectable;
	}

	const glm::mat4& Vob::getTrafoLocal() const
	{
		return mLocalTrafo;
	}

	const glm::mat4& Vob::getTrafoWorld() const
	{
		return mWorldTrafo;
	}

	const glm::mat4& Vob::getTrafoPrevWorld() const
	{
		return mPrevWorldTrafo;
	}

	void Vob::inheritParentScale(bool inherit)
	{
		mInheritParentScale = inherit;
	}

	bool Vob::isDeletable() const
	{
		return mIsDeletable;
	}

	bool Vob::isParentScaleInherited() const
	{
		return mInheritParentScale;
	}

	bool Vob::isRoot() const
	{
		return mParent == nullptr;
	}

	void Vob::rotateGlobal(const glm::vec3& axisWorld, float angle)
	{
		mRotation = glm::normalize(glm::rotate(mRotation, angle, inverse(mRotation) * axisWorld));
	}

	void Vob::rotateGlobal(const glm::vec3& eulerAngles)
	{
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.x, inverse(mRotation) * glm::vec3(1, 0, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.y, inverse(mRotation) * glm::vec3(0, 1, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.z, inverse(mRotation) * glm::vec3(0, 0, 1.0f)));
	}

	void Vob::rotateLocal(const glm::vec3& eulerAngles)
	{
		mLocalTrafo = glm::rotate(mLocalTrafo, eulerAngles.x, glm::vec3(1, 0, 0));
		mLocalTrafo = glm::rotate(mLocalTrafo, eulerAngles.y, glm::vec3(0, 1, 0));
		mLocalTrafo = glm::rotate(mLocalTrafo, eulerAngles.z, glm::vec3(0, 0, 1.0f));
	}

	void Vob::setBatches(std::list<MeshBatch>* batches)
	{
		mBatches = batches;
		recalculateLocalBoundingBox();
	}

	void Vob::setDeletable(bool deletable)
	{
		mIsDeletable = deletable;
	}

	void Vob::setOrientationLocal(const glm::vec3& eulerAngles)
	{
		const auto rotX = glm::normalize(glm::rotate(glm::quat(), eulerAngles.x, glm::vec3(1, 0, 0)));
		const auto rotY = glm::normalize(glm::rotate(glm::quat(), eulerAngles.y, glm::vec3(0, 1, 0)));
		const auto rotZ = glm::normalize(glm::rotate(glm::quat(), eulerAngles.z, glm::vec3(0, 0, 1.0f)));
		glm::quat rot = rotZ * rotY * rotX;
		setRotationLocal(rot);
	}

	void Vob::setParent(Vob* parent)
	{
		mParent = parent;
	}

	void Vob::setRotationLocal(const glm::mat4& rotation)
	{
		mRotation = glm::toQuat(rotation);
	}

	void Vob::setRotationLocal(const glm::quat& rotation)
	{
		mRotation = rotation;
	}

	void Vob::setPositionLocal(const glm::vec3& position)
	{
		mPosition = position;
	}

	void Vob::setPositionWorld(const glm::vec3& position)
	{
		auto diff = getPositionWorld() - getPositionLocal();
		setPositionLocal(position - diff);
	}

	void Vob::setScaleLocal(const glm::vec3& scale)
	{
		mScale = scale;
	}

	void Vob::setSelectable(bool selectable)
	{
		mSelectable = selectable;
	}

	void Vob::setTrafoLocal(const glm::mat4& mat)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(mat, mScale, mRotation, mPosition, skew, perspective);
		mLocalTrafo = mat;
	}

	void Vob::updateTrafo(bool resetPrevWorldTrafo, bool recalculateBoundingBox)
	{
		const auto temp = glm::mat4();
		const auto rotation = toMat4(mRotation);
		const auto scaleMat = scale(temp, mScale);
		const auto transMat = translate(temp, mPosition);
		mLocalTrafo = transMat * rotation * scaleMat;
		updateWorldTrafoHierarchy(resetPrevWorldTrafo);

		if (recalculateBoundingBox)
			recalculateBoundingBoxWorld();
	}

	void Vob::updateWorldTrafoHierarchy(bool resetPrevWorldTrafo)
	{
		updateWorldTrafo(resetPrevWorldTrafo);

		for (auto* child : mChildren) {
			child->updateWorldTrafoHierarchy(resetPrevWorldTrafo);
		}
	}

	void Vob::recalculateBoundingBoxWorld()
	{
		mBoundingBoxWorld = mWorldTrafo * mBoundingBoxLocal;
	}

	void Vob::recalculateLocalBoundingBox()
	{
		mBoundingBoxLocal = AABB();

		if (!mBatches) return;

		for (auto& batch : *mBatches) {
			mBoundingBoxLocal = maxAABB(mBoundingBoxLocal, batch.getBoundingBox());
		}
	}
	
	void Vob::updateWorldTrafo(bool resetPrevWorldTrafo)
	{
		if (!resetPrevWorldTrafo)
		{
			mPrevWorldTrafo = mWorldTrafo;
		}

		if (mParent)
		{
			if (mInheritParentScale) {
				mWorldTrafo = mParent->mWorldTrafo * mLocalTrafo;
			}
			else {
				const auto& parentTrafo = mParent->mWorldTrafo;
				auto parentScale = glm::vec3(length(parentTrafo[0]), length(parentTrafo[1]), length(parentTrafo[2]));
				mWorldTrafo = mParent->mWorldTrafo * glm::scale(mLocalTrafo, 1.0f / parentScale);
			}
			
		}
		else
		{
			mWorldTrafo = mLocalTrafo;
		}

		if (resetPrevWorldTrafo)
			mPrevWorldTrafo = mWorldTrafo;
	}

	MeshOwningVob::MeshOwningVob(Vob* parent, std::unique_ptr<MeshGroup> container) : 
		Vob(parent)
	{
		mName = "Mesh owning vob";
		mTypeName = "Mesh owning vob";
		setMeshContainer(std::move(container));
	}
	void MeshOwningVob::setMeshContainer(std::unique_ptr<MeshGroup> container)
	{
		mContainer = std::move(container);
		if (mContainer)
			setBatches(mContainer->getBatches());
	}
	MeshGroup* MeshOwningVob::getMesh() const
	{
		return mContainer.get();
	}
	MeshOwningVob::~MeshOwningVob() = default;


	RiggedVob::RiggedVob(Vob* parent) : Vob(parent), mAnimationTime(0.0f)
	{
		mName = "Rigged vob";
		mTypeName = "Rigged vob";
	}
	
	RiggedVob::~RiggedVob() = default;

	void RiggedVob::collectRenderCommands(RenderCommandQueue & queue, bool doCulling, ShaderStorageBuffer * boneTrafoBuffer)
	{
		if (!mBatches) return;

		RenderCommand command;

		for (const auto& batch : *mBatches) {
			command.batch = &batch;
			command.worldTrafo = &mWorldTrafo;
			command.prevWorldTrafo = &mPrevWorldTrafo;
			command.boundingBox = &mBoundingBoxWorld;

			command.isBoneAnimated = true;
			command.bones = &mBoneTrafos;
			command.boneBuffer = boneTrafoBuffer;

			queue.push(command, doCulling);
		}
	}
	
	void RiggedVob::frameUpdate(const RenderContext& constants)
	{
		if (mActiveAnimation == nullptr) return;
		
		updateTime(constants.frameTime);
		
		mActiveAnimation->calcBoneTrafo(mAnimationTime, mBoneTrafos);
		mActiveAnimation->applyParentHierarchyTrafos(mBoneTrafos);
	}

	const std::vector<glm::mat4>& RiggedVob::getBoneTrafos() const
	{
		return mBoneTrafos;
	}

	void RiggedVob::setActiveAnimation(const std::string& animationName)
	{
		auto* ani = AnimationManager::get()->getBoneAnimation(SID(animationName));
		setActiveAnimation(ani);
	}

	void RiggedVob::setActiveAnimation(const BoneAnimation* animation)
	{
		// Ensure that the rig of the animation matches the rig of the skinned mesh
		if (animation->getRig() != mRig) {
			throw_with_trace(std::invalid_argument("RiggedVob::setActiveAnimation: Rig of new animation doesn't match the rig of the skinned mesh!"));
		}

		mActiveAnimation = animation;
		mAnimationTime = 0.0f;
	}

	void RiggedVob::setRepeatType(AnimationRepeatType type)
	{
		mRepeatType = type;
	}

	void RiggedVob::setBatches(std::list<MeshBatch>* batches)
	{
		if (!batches) {
			Vob::setBatches(nullptr);
			return;
		}

		auto* skinnedMesh = dynamic_cast<const SkinnedMesh*>(findFirstLegalMesh(batches));

		if (skinnedMesh == nullptr) {
			throw_with_trace(std::invalid_argument("RiggedVob::setBatches: batches is expected to contain at least one SkinnedMesh instance!"));
		}

		auto id = skinnedMesh->getRigID();
		mRig = AnimationManager::get()->getBySID(SID(id));

		Vob::setBatches(batches);
	}

	const Mesh* RiggedVob::findFirstLegalMesh(std::list<MeshBatch>* batches)
	{
		for (const auto& batch : *batches) {
			for (auto& pair : batch.getEntries()) {
				if (pair.first) return pair.first;
			}
		}

		return nullptr;
	}

	void RiggedVob::updateTime(float frameTime)
	{
		mAnimationTime += frameTime;
		float duration = mActiveAnimation->getDuration();

		if (mAnimationTime <= duration) return;

		switch (mRepeatType) {
		case AnimationRepeatType::END:
				mAnimationTime = duration;
				break;
		case AnimationRepeatType::LOOP: {
			int multiplicatives = static_cast<int>(mAnimationTime / duration);
			mAnimationTime = mAnimationTime - multiplicatives * duration;
			break;
		}
		}
	}


	Billboard::Billboard(Vob* parent, Vob* child) : Vob(parent)
	{
		addChild(child);
		mName = "Billboard vob";
		mTypeName = "Billboard vob";
	}

	Billboard::Billboard(Vob* parent) : Vob(parent)
	{
	}
	
	void Billboard::frameUpdate(const RenderContext& constants)
	{
		return;	
		const auto& view = constants.camera->getView();
		
		auto view3x3 = inverse(glm::mat3(view));
		//view3x3[1] = glm::vec3(0.0f,1.0f,0.0f);
		//view3x3[0][1] = 0.0f;
		//view3x3[2][1] = 0.0f;
		auto viewRotation = glm::toQuat(view3x3);

		setRotationLocal(viewRotation);
		updateTrafo();
	}
}