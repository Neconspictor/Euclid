#include <nex/gui/Picker.hpp>
#include <nex/mesh/MeshFactory.hpp>
#include "nex/mesh/MeshGroup.hpp"
#include "nex/effects/SimpleColorPass.hpp"
#include "nex/scene/Scene.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <queue>
#include <iostream>
#include "nex/math/Ray.hpp"
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/effects/EffectLibrary.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/GI/Probe.hpp>
#include <nex/shader/ShaderProvider.hpp>
#include <nex/gui/ImGUI_Extension.hpp>

nex::gui::Picker::Picker() :
mBoundingBoxMesh(std::make_unique<MeshGroup>()),
//mLineMesh(std::make_unique<MeshGroup>()), 
mSimpleColorPass(std::make_unique<SimpleColorPass>()),
mBoundingBoxVob(nullptr)
{

	mSimpleColorPass->bind();
	mSimpleColorPass->setColor(glm::vec4(2.0f, 2.0f, 2.0f, 1.0f));

	auto shaderProvider = std::make_shared<ShaderProvider>(mSimpleColorPass.get());

	auto boxMaterial = std::make_unique<SimpleColorMaterial>(shaderProvider);
	boxMaterial->getRenderState().fillMode = FillMode::LINE;
	boxMaterial->getRenderState().doCullFaces = false;
	boxMaterial->getRenderState().doShadowCast = false;
	boxMaterial->getRenderState().isTool = true;
	boxMaterial->getRenderState().toolDrawIndex = 0;
	boxMaterial->setColor(glm::vec4(1.0f));


	auto lineMaterial = std::make_unique<SimpleColorMaterial>(shaderProvider);
	lineMaterial->getRenderState().fillMode = FillMode::LINE;
	lineMaterial->getRenderState().doCullFaces = false;
	lineMaterial->getRenderState().doShadowCast = false;
	lineMaterial->getRenderState().isTool = true;
	lineMaterial->getRenderState().toolDrawIndex = 0;
	lineMaterial->setColor(glm::vec4(1.0f));
	
	AABB box = { glm::vec3(-1.0f), glm::vec3(1.0f) };

	mBoundingBoxMesh->add(std::make_unique<MeshAABB>(box, Topology::LINES), std::move(boxMaterial));
	mBoundingBoxMesh->calcBatches();
	mBoundingBoxMesh->finalize();
	mBoundingBoxVob = std::make_unique<Vob>();
	mBoundingBoxVob->setMeshGroup(nex::make_not_owning(mBoundingBoxMesh.get()));
	mBoundingBoxVob->setSelectable(false);




	auto probeBoxMeshContainer = std::make_unique<MeshGroup>();
	auto probeBoxMaterial = RenderBackend::get()->getEffectLibrary()->createSimpleColorMaterial();

	probeBoxMaterial->setColor(glm::vec4(0.0f, 0.0f, 1.0f, 0.2f));
	probeBoxMaterial->getRenderState().blendDesc = BlendDesc::createAlphaTransparency();
	probeBoxMaterial->getRenderState().blendDesc = { BlendFunc::SOURCE_ALPHA, BlendFunc::ONE_MINUS_SOURCE_ALPHA, BlendOperation::ADD };
	probeBoxMaterial->getRenderState().doCullFaces = false;
	probeBoxMaterial->getRenderState().doDepthTest = true;
	probeBoxMaterial->getRenderState().doShadowCast = false;
	probeBoxMaterial->getRenderState().doShadowReceive = false;
	auto* meshManager = MeshManager::get();

	probeBoxMeshContainer->addMapping(meshManager->getUnitBoundingBoxTriangles(), probeBoxMaterial.get());
	probeBoxMeshContainer->addMaterial(std::move(probeBoxMaterial));
	probeBoxMeshContainer->calcBatches();
	probeBoxMeshContainer->finalize();
	mProbeInfluenceBoundingBoxVob = std::make_unique<Vob>();
	mProbeInfluenceBoundingBoxVob->setMeshGroup(std::move(probeBoxMeshContainer));
	mProbeInfluenceBoundingBoxVob->setSelectable(false);

	auto sphereMeshContainer = std::make_unique<MeshGroup>();
	auto sphereMaterial = RenderBackend::get()->getEffectLibrary()->createSimpleColorMaterial();
	sphereMaterial->setColor(glm::vec4(0.0f, 0.0f, 1.0f, 0.2f));
	sphereMaterial->getRenderState().blendDesc = BlendDesc::createAlphaTransparency();
	//sphereMaterial->getRenderState().blendDesc = { BlendFunc::SOURCE_ALPHA, BlendFunc::ONE_MINUS_SOURCE_ALPHA, BlendOperation::ADD };
	sphereMaterial->getRenderState().doCullFaces = false;
	sphereMaterial->getRenderState().doDepthTest = true;
	sphereMaterial->getRenderState().doDepthWrite = false;

	sphereMaterial->getRenderState().doShadowCast = false;
	sphereMaterial->getRenderState().doShadowReceive = false;

	sphereMeshContainer->addMapping(meshManager->getUnitSphereTriangles(), sphereMaterial.get());
	sphereMeshContainer->addMaterial(std::move(sphereMaterial));
	sphereMeshContainer->calcBatches();
	sphereMeshContainer->finalize();
	mProbeInfluenceSphereVob = std::make_unique<Vob>();
	mProbeInfluenceSphereVob->setMeshGroup(std::move(sphereMeshContainer));
	mProbeInfluenceSphereVob->setSelectable(false);

	
	//mLineMesh->add(createLineMesh(), std::move(lineMaterial));
	//mLineNode = mLineMesh->createNodeHierarchy(mNodeGeneratorScene.get());
	//mLineNode->setSelectable(false);
}

nex::gui::Picker::~Picker() = default;

nex::gui::Picker::PickedChangedCallback::Handle nex::gui::Picker::addPickedChangeCallback(const PickedChangedCallback::Callback & callback)
{
	return mCallbacks.addCallback(callback);
}

void nex::gui::Picker::removePickedChangeCallback(const PickedChangedCallback::Handle& handle)
{
	mCallbacks.removeCallback(handle);
}



void nex::gui::Picker::deselect(Scene& scene)
{
	scene.acquireLock();
	scene.removeActiveVobUnsafe(mBoundingBoxVob.get());
	scene.removeActiveVobUnsafe(mProbeInfluenceBoundingBoxVob.get());
	scene.removeActiveVobUnsafe(mProbeInfluenceSphereVob.get());
	mSelected.vob = nullptr;
}

void nex::gui::Picker::select(Scene& scene, Vob* vob)
{
	deselect(scene);

	{
		auto lock = scene.acquireLock();
		mSelected.vob = vob;
		updateBoundingBoxTrafo();
		scene.addActiveVobUnsafe(mBoundingBoxVob.get());

		if (auto* probeVob = dynamic_cast<ProbeVob*>(mSelected.vob)) {
			auto* probe = probeVob->getProbe();

			if (probe->getInfluenceType() == Probe::InfluenceType::SPHERE) {
				scene.addActiveVobUnsafe(mProbeInfluenceSphereVob.get());
			}
			else {
				scene.addActiveVobUnsafe(mProbeInfluenceBoundingBoxVob.get());
			}
		}
	}

	for (const auto& callback : mCallbacks.getCallbacks()) {
		(*callback)(mSelected.vob);
	}

}

bool nex::gui::Picker::getShowBoundingBox() const
{
	return mShowBoundingBox;
}

void nex::gui::Picker::setShowBoundingBox(bool show)
{
	mShowBoundingBox = show;
	mBoundingBoxVob->setIsVisible(show);
}


nex::Vob* nex::gui::Picker::pick(Scene& scene, const Ray& screenRayWorld, bool pickParentRoot)
{
	scene.acquireLock();

	size_t intersections = 0;
	SelectionTest selected = mSelected;

	if (!checkIntersection(selected.vob, screenRayWorld)) {
		selected.vob = nullptr;
	}

	for (const auto& root : scene.getActiveVobsUnsafe())
	{
		if (!root->getSelectable() || !root->isVisible()) continue;

		const auto invModel = inverse(root->getTrafoLocalToWorld());
		const auto origin = glm::vec3(invModel * glm::vec4(screenRayWorld.getOrigin(), 1.0f));
		const auto direction = glm::vec3(invModel * glm::vec4(screenRayWorld.getDir(), 0.0f));
		const auto rayLocal = Ray(origin, direction);
		//const auto box = node->getWorldTrafo() * root->getBoundingBox();
		const auto box = root->getBoundingBoxLocal();
		const auto result = box.testRayIntersection(rayLocal);
		if (result.intersected && (result.firstIntersection >= 0 || result.secondIntersection >= 0))
		{
			++intersections;
			const auto boundingBoxOrigin = (box.max + box.min)/2.0f;
			//root->getPosition();
			//const auto distance = length(boundingBoxOrigin - screenRayWorld.getOrigin());
			const auto distance = length(boundingBoxOrigin - rayLocal.getOrigin());
			//const auto rayMinDistance = screenRayWorld.calcClosestDistance(root->getPosition()).distance;
			const auto rootPositionLocal = glm::vec3(invModel* glm::vec4(root->getPositionLocalToParent(), 1.0f));
			const auto rayMinDistance = rayLocal.calcClosestDistance(rootPositionLocal).distance;
			const auto volume = calcVolume(box);

			SelectionTest current;
			current.vob = root;
			current.vobDistance = distance;
			current.vobRayMinDistance = rayMinDistance;
			current.vobVolume = volume;

			if (compare(current, selected) < 0) {
				if (current.vob != mSelected.vob)
					selected = current;
			}
		}
	}

	deselect(scene);

	//std::cout << "Total intersections = " << intersections << std::endl;
	if (intersections > 0)
	{
		if (pickParentRoot && selected.vob) {
			selected.vob = selected.vob->getRoot();
		}


		select(scene, selected.vob);
	}

	return mSelected.vob;
}

nex::Vob* nex::gui::Picker::getPicked()
{
	return mSelected.vob;
}

void nex::gui::Picker::updateBoundingBoxTrafo()
{
	if (!mSelected.vob) return;
	

	mSelected.vob->updateTrafo(true);

	auto* vob = mSelected.vob;
	const auto& box = vob->getBoundingBoxLocal();
	//const auto& box = vob->getBoundingBoxWorld();

	if (mUseLocalBoudningBox) {
		auto boxScaleLocal = (box.max - box.min) / 2.0f;
		auto boxOriginLocal = (box.max + box.min) / 2.0f;
		const auto objectTrafo = glm::translate(glm::mat4(), boxOriginLocal) * glm::scale(glm::mat4(), boxScaleLocal);
		const auto trafo = vob->getTrafoLocalToWorld();
		mBoundingBoxVob->setTrafoMeshToLocal(objectTrafo);
		mBoundingBoxVob->setTrafoLocalToParent(trafo);
	}
	else {
		const auto& worldBox = vob->getBoundingBoxWorld();
		auto boxOrigin = (worldBox.max + worldBox.min) / 2.0f;
		auto boxScale = (worldBox.max - worldBox.min) / 2.0f;
		const auto objectTrafo = glm::translate(glm::mat4(), boxOrigin) * glm::scale(glm::mat4(), boxScale);
		mBoundingBoxVob->setTrafoMeshToLocal(objectTrafo);
	}
	
	mBoundingBoxVob->updateWorldTrafo(true);
	mBoundingBoxVob->recalculateBoundingBoxWorld();

	//mBoundingBoxVob->getMeshRootNode()->setLocalTrafo(trafo);
	//mBoundingBoxVob->getMeshRootNode()->updateWorldTrafoHierarchy(true);


	if (auto* probeVob = dynamic_cast<ProbeVob*>(mSelected.vob)) {
		auto* probe = probeVob->getProbe();

		if (probe->getInfluenceType() == Probe::InfluenceType::SPHERE) {
			mProbeInfluenceSphereVob->setPositionLocalToParent(mSelected.vob->getPositionLocalToParent());
			mProbeInfluenceSphereVob->setScaleLocalToParent(glm::vec3(probe->getInfluenceRadius()));

			mProbeInfluenceSphereVob->updateTrafo(true);
		}
		else {
			mProbeInfluenceBoundingBoxVob->setPositionLocalToParent(mSelected.vob->getPositionLocalToParent());

			const auto& box = probe->getInfluenceBox();
			auto scale = maxVec(resolveInfinity((box.max - box.min) / 2.0f, 0.0f), glm::vec3(0.0f));
			mProbeInfluenceBoundingBoxVob->setScaleLocalToParent(scale);

			mProbeInfluenceBoundingBoxVob->updateTrafo(true);
		}

	}

	//mBoundingBoxVob->setPosition(boxOrigin);
	//mBoundingBoxVob->setScale(boxScale);
	//mBoundingBoxVob->updateTrafo();
}

std::unique_ptr<nex::Mesh> nex::gui::Picker::createBoundingBoxMesh()
{
	//create vertices in CCW
	VertexPosition vertices[8];

	// bottom plane
	vertices[0].position = glm::vec3(-1);
	vertices[1].position = glm::vec3(-1, -1, 1);
	vertices[2].position = glm::vec3(1, -1, 1);
	vertices[3].position = glm::vec3(1, -1, -1);

	// top plane
	vertices[4].position = glm::vec3(-1, 1, -1);
	vertices[5].position = glm::vec3(-1, 1, 1);
	vertices[6].position = glm::vec3(1);
	vertices[7].position = glm::vec3(1, 1, -1);

	unsigned indices[36];

	// bottom plane
	indices[0] = 0;
	indices[1] = 3;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 1;

	// front plane
	indices[6] = 0;
	indices[7] = 1;
	indices[8] = 5;
	indices[9] = 0;
	indices[10] = 5;
	indices[11] = 4;

	// back plane
	indices[12] = 3;
	indices[13] = 2;
	indices[14] = 6;
	indices[15] = 3;
	indices[16] = 6;
	indices[17] = 7;

	// left plane
	indices[18] = 0;
	indices[19] = 3;
	indices[20] = 7;
	indices[21] = 0;
	indices[22] = 7;
	indices[23] = 4;

	// right plane
	indices[24] = 1;
	indices[25] = 2;
	indices[26] = 6;
	indices[27] = 1;
	indices[28] = 6;
	indices[29] = 5;

	// top plane
	indices[30] = 4;
	indices[31] = 5;
	indices[32] = 6;
	indices[33] = 4;
	indices[34] = 6;
	indices[35] = 7;

	auto mesh = MeshFactory::createPosition(vertices, 8, indices, 36, { glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX)});
	mesh->mDebugName = "Picker::BoundingBox Triangle Mesh";

	return mesh;
}


std::unique_ptr<nex::Mesh> nex::gui::Picker::createLineMesh()
{
	//create vertices in CCW
	VertexPosition vertices[2];

	vertices[0].position = glm::vec3(0.0f);
	vertices[1].position = glm::vec3(1.0f);

	unsigned indices[2];

	// bottom plane
	indices[0] = 0;
	indices[1] = 1;

	auto mesh = MeshFactory::createPosition(vertices, 2, indices, 2, { glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX) });
	mesh->setTopology(Topology::LINES);
	mesh->mDebugName = "Picker::Line";
	return mesh;
}

float nex::gui::Picker::calcVolume(const nex::AABB & box)
{
	auto diff = box.max - box.min;
	return length(diff);
}

int nex::gui::Picker::compare(const SelectionTest & a, const SelectionTest & b)
{
	if (a.vob == nullptr) return 1;
	if (b.vob == nullptr) return -1;

	// volume has a higher priority
	if (a.vobVolume < b.vobVolume) return -1;
	if (a.vobDistance < b.vobDistance) return -1;
	if (a.vobRayMinDistance < b.vobRayMinDistance) return -1;
	if (b.vob == mSelected.vob) return -1;

	return 1;
}

bool nex::gui::Picker::checkIntersection(const Vob * vob, const nex::Ray & ray)
{
	if (!vob) return false;

	//const auto invModel = inverse(node->getWorldTrafo());
	const auto origin = glm::vec3(glm::vec4(ray.getOrigin(), 1.0f));
	const auto direction = glm::vec3(glm::vec4(ray.getDir(), 0.0f));
	const auto rayLocal = Ray(origin, direction);
	const auto& box = vob->getBoundingBoxWorld();
	const auto result = box.testRayIntersection(rayLocal);
	return (result.intersected && (result.firstIntersection >= 0 || result.secondIntersection >= 0));
}

nex::gui::Picker_View::Picker_View(Picker* picker) : mPicker(picker)
{
}

void nex::gui::Picker_View::drawSelf()
{
	nex::gui::Separator(2.0f);
	ImGui::Text("Picker:");
	bool showBoundingBox = mPicker->getShowBoundingBox();
	if (ImGui::Checkbox("Show bounding box", &showBoundingBox)) {
		mPicker->setShowBoundingBox(showBoundingBox);
	}
	nex::gui::Separator(2.0f);
}