#include <nex/gui/Picker.hpp>
#include <nex/mesh/MeshFactory.hpp>
#include "nex/mesh/StaticMesh.hpp"
#include "nex/shader/SimpleColorPass.hpp"
#include "nex/shader/Technique.hpp"
#include "nex/Scene.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <queue>
#include <iostream>
#include "nex/math/Ray.hpp"
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/EffectLibrary.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/pbr/PbrProbe.hpp>

nex::gui::Picker::Picker() :
mBoundingBoxMesh(std::make_unique<StaticMeshContainer>()),
//mLineMesh(std::make_unique<StaticMeshContainer>()), 
mSimpleColorPass(std::make_unique<SimpleColorPass>()),
mSimpleColorTechnique(std::make_unique<Technique>(mSimpleColorPass.get())),
mBoundingBoxVob(nullptr)
{

	mSimpleColorPass->bind();
	mSimpleColorPass->setColor(glm::vec4(2.0f, 2.0f, 2.0f, 1.0f));

	auto boxMaterial = std::make_unique<Material>(mSimpleColorTechnique.get());
	boxMaterial->getRenderState().fillMode = FillMode::LINE;
	boxMaterial->getRenderState().doCullFaces = false;
	boxMaterial->getRenderState().doShadowCast = false;
	boxMaterial->getRenderState().isTool = true;
	boxMaterial->getRenderState().toolDrawIndex = 0;


	auto lineMaterial = std::make_unique<Material>(mSimpleColorTechnique.get());
	lineMaterial->getRenderState().fillMode = FillMode::LINE;
	lineMaterial->getRenderState().doCullFaces = false;
	lineMaterial->getRenderState().doShadowCast = false;
	lineMaterial->getRenderState().isTool = true;
	lineMaterial->getRenderState().toolDrawIndex = 0;
	
	AABB box = { glm::vec3(-1.0f), glm::vec3(1.0f) };

	mBoundingBoxMesh->add(std::make_unique<MeshAABB>(box, Topology::LINES), std::move(boxMaterial));
	mBoundingBoxMesh->finalize();
	mBoundingBoxVob = std::make_unique<Vob>(mBoundingBoxMesh->createNodeHierarchyUnsafe());
	mBoundingBoxVob->setSelectable(false);




	auto probeBoxMeshContainer = std::make_unique<StaticMeshContainer>();
	auto probeBoxMaterial = RenderBackend::get()->getEffectLibrary()->createSimpleColorMaterial();

	probeBoxMaterial->setColor(glm::vec4(0.0f, 0.0f, 1.0f, 0.2f));
	probeBoxMaterial->getRenderState().blendDesc = BlendDesc::createAlphaTransparency();
	probeBoxMaterial->getRenderState().blendDesc = { BlendFunc::SOURCE_ALPHA, BlendFunc::ONE_MINUS_SOURCE_ALPHA, BlendOperation::ADD };
	probeBoxMaterial->getRenderState().doCullFaces = false;
	probeBoxMaterial->getRenderState().doDepthTest = true;
	probeBoxMaterial->getRenderState().doShadowCast = false;
	probeBoxMaterial->getRenderState().doShadowReceive = false;
	auto* meshManager = StaticMeshManager::get();

	probeBoxMeshContainer->addMapping(meshManager->getUnitBoundingBoxTriangles(), probeBoxMaterial.get());
	probeBoxMeshContainer->addMaterial(std::move(probeBoxMaterial));
	probeBoxMeshContainer->finalize();
	mProbeInfluenceBoundingBoxVob = std::make_unique<MeshOwningVob>(std::move(probeBoxMeshContainer));
	mProbeInfluenceBoundingBoxVob->setSelectable(false);

	auto sphereMeshContainer = std::make_unique<StaticMeshContainer>();
	auto sphereMaterial = RenderBackend::get()->getEffectLibrary()->createSimpleColorMaterial();
	sphereMaterial->setColor(glm::vec4(0.0f, 0.0f, 1.0f, 0.2f));
	sphereMaterial->getRenderState().blendDesc = BlendDesc::createAlphaTransparency();
	//sphereMaterial->getRenderState().blendDesc = { BlendFunc::SOURCE_ALPHA, BlendFunc::ONE_MINUS_SOURCE_ALPHA, BlendOperation::ADD };
	sphereMaterial->getRenderState().doCullFaces = false;
	sphereMaterial->getRenderState().doDepthTest = true;

	sphereMaterial->getRenderState().doShadowCast = false;
	sphereMaterial->getRenderState().doShadowReceive = false;

	sphereMeshContainer->addMapping(meshManager->getUnitSphereTriangles(), sphereMaterial.get());
	sphereMeshContainer->addMaterial(std::move(sphereMaterial));
	sphereMeshContainer->finalize();
	mProbeInfluenceSphereVob = std::make_unique<MeshOwningVob>(std::move(sphereMeshContainer));
	mProbeInfluenceSphereVob->setSelectable(false);

	
	//mLineMesh->add(createLineMesh(), std::move(lineMaterial));
	//mLineNode = mLineMesh->createNodeHierarchy(mNodeGeneratorScene.get());
	//mLineNode->setSelectable(false);
}

nex::gui::Picker::~Picker() = default;

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

	mSelected.vob = vob;
	updateBoundingBoxTrafo();
	scene.addActiveVobUnsafe(mBoundingBoxVob.get());

	if (mSelected.vob->getType() == VobType::Probe) {
		auto* probeVob = (ProbeVob*)mSelected.vob;
		auto* probe = probeVob->getProbe();

		if (probe->getInfluenceType() == PbrProbe::InfluenceType::SPHERE) {
			scene.addActiveVobUnsafe(mProbeInfluenceSphereVob.get());
		}
		else {
			scene.addActiveVobUnsafe(mProbeInfluenceBoundingBoxVob.get());
		}

	}
}


nex::Vob* nex::gui::Picker::pick(Scene& scene, const Ray& screenRayWorld)
{
	std::queue<SceneNode*> queue;

	scene.acquireLock();

	size_t intersections = 0;
	SelectionTest selected = mSelected;

	if (!checkIntersection(selected.vob, screenRayWorld)) {
		selected.vob = nullptr;
	}

	for (const auto& root : scene.getActiveVobsUnsafe())
	{
		if (!root->getSelectable()) continue;

		const auto& node = root->getMeshRootNode();
		if (node != nullptr)
		{
			const auto invModel = inverse(node->getWorldTrafo());
			const auto origin = glm::vec3(invModel * glm::vec4(screenRayWorld.getOrigin(), 1.0f));
			const auto direction = glm::vec3(invModel * glm::vec4(screenRayWorld.getDir(), 0.0f));
			const auto rayLocal = Ray(origin, direction);
			//const auto box = node->getWorldTrafo() * root->getBoundingBox();
			const auto box = root->getBoundingBox();
			const auto result = box.testRayIntersection(rayLocal);
			if (result.intersected && (result.firstIntersection >= 0 || result.secondIntersection >= 0))
			{
				++intersections;
				const auto boundingBoxOrigin = (box.max + box.min)/2.0f;
				//root->getPosition();
				//const auto distance = length(boundingBoxOrigin - screenRayWorld.getOrigin());
				const auto distance = length(boundingBoxOrigin - rayLocal.getOrigin());
				//const auto rayMinDistance = screenRayWorld.calcClosestDistance(root->getPosition()).distance;
				const auto rootPositionLocal = glm::vec3(invModel* glm::vec4(root->getPosition(), 1.0f));
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
	}

	deselect(scene);

	//std::cout << "Total intersections = " << intersections << std::endl;
	if (intersections > 0)
	{
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

	auto* node = mSelected.vob->getMeshRootNode();
	const auto& box = mSelected.vob->getBoundingBox();


	const auto worldBox = node->getWorldTrafo() * box;
	auto boxOrigin = (worldBox.max + worldBox.min) / 2.0f;
	auto boxScale = (worldBox.max - worldBox.min) / 2.0f;
	auto boxScaleLocal = (box.max - box.min) / 2.0f;
	auto boxOriginLocal = (box.max + box.min) / 2.0f;

	const auto objectTrafo = glm::translate(glm::mat4(), boxOriginLocal) * glm::scale(glm::mat4(), boxScaleLocal);
	const auto trafo = node->getWorldTrafo() * objectTrafo;

	mBoundingBoxVob->getMeshRootNode()->setLocalTrafo(trafo);
	mBoundingBoxVob->getMeshRootNode()->updateWorldTrafoHierarchy(true);


	if (mSelected.vob->getType() == VobType::Probe) {
		auto* probeVob = (ProbeVob*)mSelected.vob;
		auto* probe = probeVob->getProbe();

		if (probe->getInfluenceType() == PbrProbe::InfluenceType::SPHERE) {
			mProbeInfluenceSphereVob->setPosition(mSelected.vob->getPosition());
			mProbeInfluenceSphereVob->setScale(glm::vec3(probe->getInfluenceRadius()));

			mProbeInfluenceSphereVob->updateTrafo(true);
		}
		else {
			mProbeInfluenceBoundingBoxVob->setPosition(mSelected.vob->getPosition());

			const auto& box = probe->getInfluenceBox();
			auto scale = maxVec(resolveInfinity((box.max - box.min) / 2.0f, 0.0f), glm::vec3(0.0f));
			mProbeInfluenceBoundingBoxVob->setScale(scale);

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
	const auto box = vob->getMeshRootNode()->getWorldTrafo() * vob->getBoundingBox();
	const auto result = box.testRayIntersection(rayLocal);
	return (result.intersected && (result.firstIntersection >= 0 || result.secondIntersection >= 0));
}