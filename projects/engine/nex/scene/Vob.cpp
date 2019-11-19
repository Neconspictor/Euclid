#include <nex/scene/Vob.hpp>
#include <nex/scene/Scene.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/math/Math.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <nex/anim/AnimationManager.hpp>

namespace nex
{
	Vob::Vob(SceneNode* meshRootNode) : mMeshRootNode(meshRootNode), mPosition(0.0f), mRotation(glm::quat()), mScale(1.0f), 
		mSelectable(true), mIsDeletable(true),
		mType(VobType::Normal)
	{
	}

	Vob::~Vob()
	{
		if (mMeshRootNode) delete mMeshRootNode;
		mMeshRootNode = nullptr;
	}

	const SceneNode* Vob::getMeshRootNode() const
	{
		return mMeshRootNode;
	}

	SceneNode* Vob::getMeshRootNode()
	{
		return mMeshRootNode;
	}

	const AABB& Vob::getBoundingBox() const
	{
		return mBoundingBox;
	}

	const glm::vec3& Vob::getPosition() const
	{
		return mPosition;
	}

	const glm::quat& Vob::getRotation() const
	{
		return mRotation;
	}

	const glm::vec3& Vob::getScale() const
	{
		return mScale;
	}

	bool Vob::getSelectable() const
	{
		return mSelectable;
	}

	VobType Vob::getType() const
	{
		return mType;
	}

	bool Vob::isDeletable() const
	{
		return mIsDeletable;
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
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.x, glm::vec3(1, 0, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.y, glm::vec3(0, 1, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.z, glm::vec3(0, 0, 1.0f)));
	}

	void Vob::setDeletable(bool deletable)
	{
		mIsDeletable = deletable;
	}

	void Vob::setMeshRootNode(SceneNode* node)
	{
		if (mMeshRootNode) delete mMeshRootNode;
		mMeshRootNode = node;
	}

	void Vob::setOrientation(const glm::vec3& eulerAngles)
	{
		const auto rotX = glm::normalize(glm::rotate(glm::quat(), eulerAngles.x, glm::vec3(1, 0, 0)));
		const auto rotY = glm::normalize(glm::rotate(glm::quat(), eulerAngles.y, glm::vec3(0, 1, 0)));
		const auto rotZ = glm::normalize(glm::rotate(glm::quat(), eulerAngles.z, glm::vec3(0, 0, 1.0f)));
		mRotation = rotZ * rotY * rotX;
	}

	void Vob::setRotation(const glm::mat4& rotation)
	{
		mRotation = rotation;
	}

	void Vob::setRotation(const glm::quat& rotation)
	{
		mRotation = rotation;
	}

	void Vob::setPosition(const glm::vec3& position)
	{
		mPosition = position;
	}

	void Vob::setScale(const glm::vec3& scale)
	{
		mScale = scale;
	}

	void Vob::setSelectable(bool selectable)
	{
		mSelectable = selectable;
	}

	void Vob::setTrafo(const glm::mat4& mat)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(mat, mScale, mRotation, mPosition, skew, perspective);
	}

	void Vob::updateTrafo(bool resetPrevWorldTrafo)
	{
		if (!mMeshRootNode) return;

		const auto temp = glm::mat4();
		const auto rotation = toMat4(mRotation);
		const auto scaleMat = scale(temp, mScale);
		const auto transMat = translate(temp, mPosition);
		mMeshRootNode->setLocalTrafo(transMat * rotation * scaleMat);
		mMeshRootNode->updateWorldTrafoHierarchy(resetPrevWorldTrafo);
		recalculateBoundingBox();
	}

	void Vob::recalculateBoundingBox()
	{
		mBoundingBox = { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };

		if (!mMeshRootNode)
		{
			return;
		}

		std::queue<SceneNode*> nodes;
		nodes.push(mMeshRootNode);

		while (!nodes.empty())
		{
			auto* node = nodes.front();
			nodes.pop();

			const auto& children = node->getChildren();
			for (auto& child : children)
				nodes.push(child);


			const auto* batch = node->getBatch();
			if (!batch) continue;
			mBoundingBox = maxAABB(mBoundingBox, batch->getBoundingBox());
		}
	}
	MeshOwningVob::MeshOwningVob(std::unique_ptr<MeshGroup> container) : 
		Vob(nullptr)
	{
		setMeshContainer(std::move(container));
	}
	void MeshOwningVob::setMeshContainer(std::unique_ptr<MeshGroup> container)
	{
		mContainer = std::move(container);
		if (mContainer)
			setMeshRootNode(mContainer->createNodeHierarchyUnsafe());
	}
	MeshGroup* MeshOwningVob::getMesh() const
	{
		return mContainer.get();
	}
	MeshOwningVob::~MeshOwningVob() = default;


	RiggedVob::RiggedVob(SceneNode* meshRootNode) : Vob(meshRootNode), mAnimationTime(0.0f)
	{
		auto* skinnedMesh = dynamic_cast<const SkinnedMesh*>(findFirstLegalMesh(meshRootNode));

		if (skinnedMesh == nullptr) {
			throw_with_trace(std::invalid_argument("RiggedVob::RiggedVob: meshRootNode is expected to contain at least one SkinnedMesh instance!"));
		}

		auto id = skinnedMesh->getRigID();
		mRig = AnimationManager::get()->getBySID(SID(id));
		mType = VobType::Skinned;
	}
	
	RiggedVob::~RiggedVob() = default;
	
	void RiggedVob::frameUpdate(float frameTime)
	{
		if (mActiveAnimation == nullptr) return;
		
		updateTime(frameTime);
		
		auto minMaxs = mActiveAnimation->calcMinMaxKeyFrames(0.0f);
		auto interpolatedTrafos = BoneAnimation::calcInterpolatedTrafo(minMaxs, 0.0f);
		minMaxs.clear();

		auto nodeTrafos = BoneAnimation::calcBoneTrafo(interpolatedTrafos);
		interpolatedTrafos.clear();
		mBoneTrafos.resize(nodeTrafos.size());

		mActiveAnimation->applyParentHierarchyTrafos(nodeTrafos, mBoneTrafos);
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

	const Mesh* RiggedVob::findFirstLegalMesh(SceneNode* node)
	{
		auto* batch = node->getBatch();

		if (batch) {
			for (auto& pair : batch->getMeshes()) {
				if (pair.first) return pair.first;
			}
		}

		for (auto* child : node->getChildren()) {
			auto* mesh = findFirstLegalMesh(child);
			if (mesh) return mesh;
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
}