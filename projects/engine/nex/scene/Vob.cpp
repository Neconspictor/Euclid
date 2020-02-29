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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <nex/scene/VobBluePrint.hpp>

namespace nex
{
	Vob::Vob() : 
		RenderCommandFactory(),
		FrameUpdateable(),
		mSelectable(true), 
		mIsDeletable(true),
		mParent(nullptr), 
		mMeshGroup(nullptr),
		mName("Normal vob"),
		mTypeName("Normal vob"),
		mInheritParentScale(true),
		mUsesPerObjectMaterialData(true)
	{
	}

	Vob::~Vob() = default;

	void Vob::addChild(ChildPtr child)
	{
		child->setParent(this);
		mChildren.emplace_back(std::move(child));
	}

	void Vob::applyTrafoLocalToWorld(const glm::mat4& trafoLocalToWorld, const glm::vec3& origin)
	{
		auto worldToOrigin = translate(glm::mat4(1.0f), -origin);
		auto originToWorld = translate(glm::mat4(1.0f), origin);

		auto toNewLocal = originToWorld * trafoLocalToWorld * worldToOrigin;

		if (mParent) {
			const auto& parentToWorld = mParent->mTrafoLocalToWorld;
			auto worldToParent = inverse(parentToWorld);
			toNewLocal = worldToParent * toNewLocal *  parentToWorld;
		}
		
		setTrafoLocalToParent(toNewLocal * mLocalToParentSpace.getTrafo());
	}

	void Vob::clearShear()
	{
		mLocalToParentSpace.setShear(glm::vec3(0.0f));
		mLocalToParentSpace.update();
	}

	void Vob::collectRenderCommands(RenderCommandQueue& queue, bool doCulling, const RenderContext& renderContext) const
	{
		if (!mMeshGroup.get()) return;

		auto* batches = mMeshGroup->getBatches();
		if (!batches) return;

		RenderCommand command;

		for (const auto& batch : *batches) {
			command.batch = &batch;
			command.worldTrafo = &mTrafoMeshToWorld;
			command.prevWorldTrafo = &mTrafoPrevMeshToWorld;
			command.boundingBox = &mBoundingBoxWorld;

			command.isBoneAnimated = false;
			command.bones = nullptr;
			command.boneBuffer = nullptr;
			command.perObjectMaterialID = mPerObjectMaterialDataID;

			queue.push(command, doCulling);
		}
	}

	nex::Vob::ChildPtr Vob::removeChild(Vob* child)
	{
		auto it = std::remove_if(mChildren.begin(), mChildren.end(), [child](const Vob::ChildPtr& current) 
		{
			return current.get() == child;
		});

		ChildPtr managedChild;

		if (it != mChildren.end()) {
			managedChild = *it;
		}

		mChildren.erase(it, mChildren.end());

		return managedChild;
	}

	std::unique_ptr<Vob> nex::Vob::createBluePrintCopy() const
	{
		auto result = createBluePrintRecursive();
		result->updateTrafo(true);
		return result;
	}

	void nex::Vob::finalizeMeshes() {
		mMeshGroup->finalize();
		for (auto& child : mChildren) {
			child->finalizeMeshes();
		}
	}

	void Vob::frameUpdate(const RenderContext& constants)
	{
		// update key frame animation
		const auto* ani = getActiveKeyFrameAnimation();
		if (!ani || mActiveKeyFrameAniData.paused) return;

		mActiveKeyFrameAniData.updateTime(constants.frameTime, ani->getDuration());

		const auto channelCount = ani->getChannelCount();
		std::vector<glm::mat4> trafos;
		ani->calcChannelTrafos(mActiveKeyFrameAniData.time, trafos);

		const auto& mapping = mBluePrint->getMapping();
		const auto& usedChannelIds = ani->getUsedChannelIDs();
		const auto& inverseLocalToParentTrafos = mBluePrint->getInverseLocalToParentTrafos();

		std::queue<nex::Vob*> queue;
		queue.push(this);
		while (!queue.empty()) {
			auto* vob = queue.front();
			queue.pop();
			const auto sid = vob->getBluePrintNodeNameSID();
			auto it = mapping.find(sid);
		
			glm::mat4 trafo;
			if (it != end(mapping)) {

				const auto index = it->second;
				auto itChannelUsed = usedChannelIds.find(index);

				if (itChannelUsed != end(usedChannelIds)) {
					const auto& inverseLocalToParent = inverseLocalToParentTrafos[index];
					trafo = trafos[index] * inverseLocalToParent;
				}
				else {
					trafo = glm::mat4(1.0f);
				}

			}
			else {

				trafo = glm::mat4(1.0f);
			}
			


			vob->setAnimationTrafo(trafo);

			// add children to queue
			for (auto& child : vob->getChildren()) {
				queue.push(child.get());
			}
		}

		updateTrafo();
	}

	const nex::VobBluePrint* Vob::getBluePrint() const
	{
		return mBluePrint;
	}

	unsigned Vob::getBluePrintNodeNameSID() const
	{
		return mBluePrintNodeNameSID;
	}

	nex::MeshGroup* Vob::getMeshGroup()
	{
		return mMeshGroup.get();
	}

	const nex::MeshGroup* Vob::getMeshGroup() const
	{
		return mMeshGroup.get();
	}

	const AABB& Vob::getBoundingBoxWorld() const
	{
		return mBoundingBoxWorld;
	}

	const nex::AABB& Vob::getBoundingBoxLocal() const
	{
		return mBoundingBoxLocal;
	}

	std::vector<Vob::ChildPtr>& Vob::getChildren()
	{
		return mChildren;
	}

	const std::vector<Vob::ChildPtr>& Vob::getChildren() const
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

	const glm::vec3& Vob::getPositionLocalToParent() const
	{
		return mLocalToParentSpace.getPosition();
	}

	const glm::vec3& Vob::getPositionLocalToWorld() const
	{
		return reinterpret_cast<const glm::vec3&>(mTrafoLocalToWorld[3]);
	}

	const glm::quat& Vob::getRotationLocalToParent() const
	{
		return mLocalToParentSpace.getRotation();
	}

	glm::quat Vob::getRotationLocalToWorld() const
	{
		glm::vec3 scale, pos, skew;
		glm::quat rotation;
		glm::vec4 persp;
		glm::decompose(mTrafoLocalToWorld, scale, rotation, pos, skew, persp);
		return rotation;
		//return glm::quat(glm::extractMatrixRotation(mTrafoLocalToWorld));
	}

	Vob* Vob::getParent()
	{
		return mParent;
	}

	const Vob* Vob::getParent() const
	{
		return mParent;
	}

	glm::vec3 Vob::getScaleLocalToWorld() const
	{
		return glm::vec3(length(mTrafoLocalToWorld[0]), length(mTrafoLocalToWorld[1]), length(mTrafoLocalToWorld[2]));
	}

	const glm::vec3& Vob::getScaleLocalToParent() const
	{
		return mLocalToParentSpace.getScale();
	}

	bool Vob::getSelectable() const
	{
		return mSelectable;
	}

	const glm::mat4& Vob::getTrafoMeshToLocal() const
	{
		return mTrafoMeshToLocal;
	}

	const glm::mat4& Vob::getTrafoLocalToParent() const
	{
		return mLocalToParentSpace.getTrafo();
	}

	const glm::mat4& Vob::getTrafoLocalToWorld() const
	{
		return mTrafoLocalToWorld;
	}

	const glm::mat4& Vob::getTrafoMeshToWorld() const
	{
		return mTrafoMeshToWorld;
	}

	const glm::mat4& Vob::getTrafoPrevMeshToWorld() const
	{
		return mTrafoPrevMeshToWorld;
	}

	void Vob::inheritParentScale(bool inherit)
	{
		mInheritParentScale = inherit;
	}

	bool Vob::hasChild(const Vob* vob) const
	{
		for (const auto& child : mChildren) {
			if (child.get() == vob || child->hasChild(vob)) return true;
		}

		return false;
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

	bool Vob::isStatic() const
	{
		return mIsStatic;
	}

	void Vob::rotateGlobal(const glm::vec3& axisWorld, float angle)
	{
		auto rotation = mLocalToParentSpace.getRotation();
		rotation = glm::normalize(glm::rotate(rotation, angle, inverse(rotation) * axisWorld));
		mLocalToParentSpace.setRotation(rotation);
	}

	void Vob::rotateGlobal(const glm::vec3& eulerAngles)
	{
		auto rotation = mLocalToParentSpace.getRotation();
		rotation = glm::normalize(glm::rotate(rotation, eulerAngles.x, inverse(rotation) * glm::vec3(1, 0, 0)));
		rotation = glm::normalize(glm::rotate(rotation, eulerAngles.y, inverse(rotation) * glm::vec3(0, 1, 0)));
		rotation = glm::normalize(glm::rotate(rotation, eulerAngles.z, inverse(rotation) * glm::vec3(0, 0, 1.0f)));
		mLocalToParentSpace.setRotation(rotation);
	}

	void Vob::rotateLocal(const glm::vec3& eulerAngles)
	{
		auto trafo = glm::rotate(mLocalToParentSpace.getTrafo(), eulerAngles.x, glm::vec3(1, 0, 0));
		trafo = glm::rotate(trafo, eulerAngles.y, glm::vec3(0, 1, 0));
		trafo = glm::rotate(trafo, eulerAngles.z, glm::vec3(0, 0, 1.0f));
		setTrafoLocalToParent(trafo);
	}

	void Vob::setActiveKeyFrameAnimation(nex::Sid sid)
	{
		if (!mBluePrint) return;

		// Is animation not applicable?
		// TODO: Throw exception?
		if (mBluePrintNodeNameSID != mBluePrint->getBluePrintRootNameSID()) {
			return;
		}

		const auto& anis = mBluePrint->getKeyFrameAnimations();

		if (mActiveKeyFrameAniSID && (anis.find(sid) == end(anis))) {
			throw_with_trace(std::invalid_argument("sid doesn't match to a stored keyframe animation!"));
		}
		
		if (mActiveKeyFrameAniSID != sid) mActiveKeyFrameAniData.reset();

		mActiveKeyFrameAniSID = sid;
	}

	const nex::KeyFrameAnimation* Vob::getActiveKeyFrameAnimation() const
	{
		if (!mBluePrint || !mActiveKeyFrameAniSID) return nullptr;
		const auto& anis = mBluePrint->getKeyFrameAnimations();

		return anis.at(mActiveKeyFrameAniSID).get();
	}

	void Vob::setAnimationTrafo(const glm::mat4& trafo)
	{
		mTrafoBeforeLocalAnimationToLocal = trafo;
	}

	void Vob::setBluePrint(const nex::VobBluePrint* bluePrint)
	{
		mBluePrint = bluePrint;
	}

	void Vob::setMeshGroup(MeshGroupPtr meshGroup)
	{
		mMeshGroup = std::move(meshGroup);
		recalculateLocalBoundingBox();
	}

	void Vob::setDeletable(bool deletable)
	{
		mIsDeletable = deletable;
	}

	void Vob::setIsStatic(bool isStatic)
	{
		mIsStatic = isStatic;
	}

	void Vob::setRotationLocalToParent(const glm::vec3& eulerAngles)
	{
		const auto rotX = glm::normalize(glm::rotate(glm::quat(), eulerAngles.x, glm::vec3(1, 0, 0)));
		const auto rotY = glm::normalize(glm::rotate(glm::quat(), eulerAngles.y, glm::vec3(0, 1, 0)));
		const auto rotZ = glm::normalize(glm::rotate(glm::quat(), eulerAngles.z, glm::vec3(0, 0, 1.0f)));
		glm::quat rot = rotZ * rotY * rotX;
		setRotationLocalToParent(rot);
	}

	void Vob::setParent(Vob* parent)
	{
		mParent = parent;
	}

	void Vob::setRotationLocalToParent(const glm::mat4& rotation)
	{
		mLocalToParentSpace.setRotation(glm::toQuat(rotation));
	}

	void Vob::setRotationLocalToParent(const glm::quat& rotation)
	{
		mLocalToParentSpace.setRotation(rotation);
	}

	void Vob::setRotationLocalToWorld(const glm::quat& rotation)
	{
		auto oldRotation = getRotationLocalToWorld();
		auto newWorldTrafo = glm::toMat4(rotation) * inverse(glm::toMat4(oldRotation));// *mTrafoLocalToWorld;
		applyTrafoLocalToWorld(newWorldTrafo, getPositionLocalToWorld());
	
	}

	void Vob::setPositionLocalToParent(const glm::vec3& position)
	{
		mLocalToParentSpace.setPosition(position);
	}

	void Vob::setPositionLocalToWorld(const glm::vec3& position)
	{
		//Note: we don't need an origin, it suffices to go to local space 
		auto worldToParent = mLocalToParentSpace.getTrafo() * inverse(mTrafoLocalToWorld);
		auto positionLocalToParent = glm::vec3(worldToParent * glm::vec4(position, 1.0f));
		setPositionLocalToParent(positionLocalToParent);
	}

	void Vob::setScaleLocalToParent(const glm::vec3& scale)
	{
		mLocalToParentSpace.setScale(scale);
	}

	void Vob::setScaleLocalToWorld(const glm::vec3& newScale, const glm::vec3& minOldScale)
	{
		glm::vec3 scale, position, skew;
		glm::vec4 perspective;
		glm::quat rotation;
		glm::decompose(mTrafoLocalToWorld, scale, rotation, position, skew, perspective);

		glm::mat4 unit(1.0f);

		glm::vec3 origin = mTrafoLocalToWorld[3];
		auto worldToOrigin = translate(glm::mat4(1.0f), -origin);
		auto originToWorld = translate(glm::mat4(1.0f), origin);

		auto oldScale = maxVec(getScaleLocalToWorld(), glm::vec3(0.0001f)); 
		auto newScaleClamped = maxVec(newScale, glm::vec3(0.0001f));

		auto scaleMatrix = glm::scale(unit, maxVec(newScaleClamped / oldScale, glm::vec3(0.0001f)));
		auto newWorld = originToWorld * scaleMatrix * worldToOrigin * mTrafoLocalToWorld;

		auto diff = newWorld * inverse(mTrafoLocalToWorld);
		applyTrafoLocalToWorld(diff);
	}

	void Vob::setSelectable(bool selectable)
	{
		mSelectable = selectable;
	}

	void Vob::setTrafoLocalToParent(const glm::mat4& mat)
	{
		mLocalToParentSpace.setTrafo(mat);
	}

	void Vob::setTrafoMeshToLocal(const glm::mat4& mat)
	{
		mTrafoMeshToLocal = mat;
	}

	const PerObjectMaterialData& Vob::getPerObjectMaterialData() const
	{
		return mPerObjectMaterialData;
	}

	nex::PerObjectMaterialData& Vob::getPerObjectMaterialData()
	{
		return mPerObjectMaterialData;
	}

	unsigned Vob::getPerObjectMaterialDataID() const
	{
		return mPerObjectMaterialDataID;
	}

	bool Vob::usesPerObjectMaterialData() const
	{
		return mUsesPerObjectMaterialData;
	}

	void Vob::usePerObjectMaterialData(bool val)
	{
		mUsesPerObjectMaterialData = val;
	}

	void Vob::setPerObjectMaterialData(const PerObjectMaterialData& data)
	{
		mPerObjectMaterialData = data;
	}

	void Vob::setPerObjectMaterialDataID(unsigned id)
	{
		mPerObjectMaterialDataID = id;
	}

	void Vob::updateTrafo(bool resetPrevWorldTrafo, bool recalculateBoundingBox)
	{
		mLocalToParentSpace.update();
		updateWorldTrafo(resetPrevWorldTrafo);
		
		for (auto& child : mChildren) {
			child->updateTrafo(resetPrevWorldTrafo, recalculateBoundingBox);
		}

		if (recalculateBoundingBox)
			recalculateBoundingBoxWorld();
	}

	void Vob::updateWorldTrafoHierarchy(bool resetPrevWorldTrafo)
	{
		updateWorldTrafo(resetPrevWorldTrafo);

		for (auto& child : mChildren) {
			child->updateWorldTrafoHierarchy(resetPrevWorldTrafo);
		}
	}

	void Vob::recalculateBoundingBoxWorld()
	{
		recalculateLocalBoundingBox();
		
		mBoundingBoxWorld = mTrafoLocalToWorld * mBoundingBoxLocal;

		for (auto& child : mChildren) {
			//child->recalculateBoundingBoxWorld();
			mBoundingBoxWorld = nex::maxAABB(mBoundingBoxWorld, child->mBoundingBoxWorld);
		}

		
	}

	void Vob::recalculateLocalBoundingBox()
	{
		mBoundingBoxLocal = AABB();

		if (mMeshGroup.get()) {
			auto* batches = mMeshGroup->getBatches();
			if (!batches) return;

			for (auto& batch : *batches) {
				mBoundingBoxLocal = maxAABB(mBoundingBoxLocal, mTrafoMeshToLocal * batch.getBoundingBox());
			}
		}
	}
	
	void Vob::updateWorldTrafo(bool resetPrevWorldTrafo)
	{
		mLocalToParentSpace.update();
		const auto trafoLocalToParent = mTrafoBeforeLocalAnimationToLocal * mLocalToParentSpace.getTrafo();// mLocalToParentSpace.getTrafo();// *mTrafoBeforeLocalAnimationToLocal;

		if (!resetPrevWorldTrafo)
		{
			mTrafoPrevMeshToWorld = mTrafoMeshToWorld;
		}

		if (mParent)
		{
			mTrafoLocalToWorld = mParent->mTrafoLocalToWorld * trafoLocalToParent;
		}
		else
		{
			mTrafoLocalToWorld = trafoLocalToParent;
		}


		if (resetPrevWorldTrafo)
			mTrafoPrevMeshToWorld = mTrafoMeshToWorld;

		mTrafoMeshToWorld = mTrafoLocalToWorld * mTrafoMeshToLocal;
	}

	std::unique_ptr<Vob> Vob::createBluePrintRecursive() const
	{
		std::unique_ptr<Vob> result = createNew();
		result->setMeshGroup(mMeshGroup);
		result->setTrafoLocalToParent(getTrafoLocalToParent());
		result->setTrafoMeshToLocal(getTrafoMeshToLocal());
		result->getName() = mName;
		result->mBluePrintNodeNameSID = SID(mName);

		for (auto& child : mChildren) {
			auto newChild = child->createBluePrintRecursive();
			result->addChild(std::move(newChild));
		}

		return result;
	}

	std::unique_ptr<Vob> Vob::createNew() const
	{
		return std::make_unique<Vob>();
	}


	void nex::Vob::AnimationData::reset() {
		time = 0.0f;
		paused = false;
		mRepeatType = AnimationRepeatType::LOOP;
	}

	void nex::Vob::AnimationData::updateTime(float frameTime, float duration) {
		
		if (paused) return;

		time += frameTime;
		if (time <= duration) return;

		switch (mRepeatType) {
		case AnimationRepeatType::END:
			time = duration;
			break;
		case AnimationRepeatType::LOOP: {
			int multiplicatives = static_cast<int>(time / duration);
			time = time - multiplicatives * duration;
			break;
		}
		}
	}


	RiggedVob::RiggedVob() : Vob(), mAnimationTime(0.0f)
	{
		mName = "Rigged vob";
		mTypeName = "Rigged vob";

		setActiveAnimation(nullptr);
	}
	
	RiggedVob::~RiggedVob() = default;

	void RiggedVob::collectRenderCommands(RenderCommandQueue & queue, bool doCulling, const RenderContext& renderContext) const
	{
		auto* group = mMeshGroup.get();

		if (!group) return;
		auto* batches = group->getBatches();
		if (!batches) return;

		RenderCommand command;

		for (const auto& batch : *batches) {
			command.batch = &batch;
			command.worldTrafo = &mTrafoMeshToWorld;
			command.prevWorldTrafo = &mTrafoPrevMeshToWorld;
			command.boundingBox = &mBoundingBoxWorld;

			command.isBoneAnimated = true;
			command.bones = &mBoneTrafos;
			command.boneBuffer = renderContext.boneTransformBuffer.get();
			command.perObjectMaterialID = mPerObjectMaterialDataID;

			queue.push(command, doCulling);
		}
	}
	
	void RiggedVob::frameUpdate(const RenderContext& constants)
	{
		Vob::frameUpdate(constants);

		if (mActiveAnimation == nullptr || mIsPaused) return;
		
		updateTime(constants.frameTime);
		
		mActiveAnimation->calcChannelTrafos(mAnimationTime, mBoneTrafos);
		mActiveAnimation->applyParentHierarchyTrafos(mBoneTrafos);



		/*if (mDefaultScale != 1.0f) {
			glm::mat4 defaultScaleTrafo (mDefaultScale);
			defaultScaleTrafo[3][3] = 1.0f;
			glm::mat4 defaultScaleInverseTrafo(1.0f / mDefaultScale);
			defaultScaleInverseTrafo[3][3] = 1.0f;
			for (auto& trafo : mBoneTrafos) {
				trafo = defaultScaleTrafo * trafo * defaultScaleInverseTrafo;
			}
		}*/
	}

	const nex::BoneAnimation* RiggedVob::getActiveAnimation() const
	{
		return mActiveAnimation;
	}

	const std::vector<glm::mat4>& RiggedVob::getBoneTrafos() const
	{
		return mBoneTrafos;
	}

	const Rig* RiggedVob::getRig() const
	{
		return mRig;
	}

	void RiggedVob::pauseAnimation(bool pause)
	{
		mIsPaused = pause;
	}

	bool RiggedVob::isAnimationPaused() const
	{
		return mIsPaused;
	}

	void RiggedVob::setActiveAnimation(const std::string& animationName)
	{
		auto* ani = AnimationManager::get()->getBoneAnimation(SID(animationName));
		setActiveAnimation(ani);
	}

	void RiggedVob::setActiveAnimation(const BoneAnimation* animation)
	{
		// Ensure that the rig of the animation matches the rig of the skinned mesh
		if (animation && animation->getRig() != mRig) {
			throw_with_trace(std::invalid_argument("RiggedVob::setActiveAnimation: Rig of new animation doesn't match the rig of the skinned mesh!"));
		}

		mActiveAnimation = animation;
		mAnimationTime = 0.0f;

		// set default bone transformations if no animation is set
		if (!mActiveAnimation && mRig) {

			mBoneTrafos.resize(mRig->getBones().size());

			auto& invRoot = mRig->getInverseRootTrafo();

			for (auto& trafo : mBoneTrafos) {
				trafo = invRoot;
			}
		}
	}

	void RiggedVob::setRepeatType(AnimationRepeatType type)
	{
		mRepeatType = type;
	}

	void RiggedVob::setMeshGroup(MeshGroupPtr meshGroup)
	{
		if (!meshGroup.get() || !meshGroup->getBatches()) {
			Vob::setMeshGroup(nullptr);
			return;
		}

		auto* batches = meshGroup->getBatches();

		auto* skinnedMesh = dynamic_cast<const SkinnedMesh*>(findFirstLegalMesh(batches));

		if (skinnedMesh == nullptr) {
			throw_with_trace(std::invalid_argument("RiggedVob::setBatches: batches is expected to contain at least one SkinnedMesh instance!"));
		}

		auto id = skinnedMesh->getRigID();
		mRig = AnimationManager::get()->getBySID(SID(id));
		if (!mRig) {
			throw_with_trace(std::runtime_error("RiggedVob::setBatches(): Rig is not a registered rig: " + id));
		}

		if (!mActiveAnimation) setActiveAnimation(nullptr);

		Vob::setMeshGroup(std::move(meshGroup));
	}

	void RiggedVob::recalculateLocalBoundingBox()
	{
		mBoundingBoxLocal = AABB();
		const auto& invRoot = mRig->getInverseRootTrafo();

		if (!mMeshGroup.get()) return;
		auto* batches = mMeshGroup->getBatches();
		if (!batches) return;

		for (auto& batch : *batches) {
			mBoundingBoxLocal = maxAABB(mBoundingBoxLocal, mTrafoMeshToLocal * invRoot * batch.getBoundingBox());
		}
	}

	std::unique_ptr<Vob> RiggedVob::createNew() const
	{
		return std::make_unique<RiggedVob>();
	}

	const Mesh* RiggedVob::findFirstLegalMesh(std::vector<MeshBatch>* batches)
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
		if (!mActiveAnimation) return;

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


	Billboard::Billboard() : Vob()
	{
		usePerObjectMaterialData(false);
		mName = "Billboard vob";
		mTypeName = "Billboard vob";
	}


	void Billboard::frameUpdate(const RenderContext& constants)
	{
		Vob::frameUpdate(constants);
		return;	
		const auto& view = constants.camera->getView();
		
		auto view3x3 = inverse(glm::mat3(view));
		//view3x3[1] = glm::vec3(0.0f,1.0f,0.0f);
		//view3x3[0][1] = 0.0f;
		//view3x3[2][1] = 0.0f;
		auto viewRotation = glm::toQuat(view3x3);

		setRotationLocalToParent(viewRotation);
		updateTrafo();
	}
	std::unique_ptr<Vob> Billboard::createNew() const
	{
		return std::make_unique<Billboard>();
	}
}