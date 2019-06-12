#include <nex/gui/Gizmo.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/shader/Technique.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/Scene.hpp>
#include <nex/mesh/Mesh.hpp>
#include "nex/math/Ray.hpp"
#include <nex/math/Math.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include "nex/camera/Camera.hpp"
#include <nex/math/Math.hpp>
#include <nex/math/Constant.hpp>

class nex::gui::Gizmo::GizmoPass : public TransformPass
{
public:
	GizmoPass() : TransformPass(Shader::create("gui/gizmo/gizmo_vs.glsl", "gui/gizmo/gizmo_fs.glsl"))
	{
		bind();
		mSelectedAxis = {mShader->getUniformLocation("selectedAxis"), UniformType::UINT};

		// Default state: No axis is selected
		setSelectedAxis(Axis::INVALID);
	}

	void setSelectedAxis(Axis axis)
	{
		mShader->setUInt(mSelectedAxis.location, (unsigned)axis);
	}

private:
	Uniform mSelectedAxis;
};

nex::gui::Gizmo::Gizmo(Mode mode) : mNodeGeneratorScene(std::make_unique<Scene>()), mTranslationGizmoNode(nullptr),
							mActivationState({ false, Axis::INVALID, glm::vec3(0.0f), glm::vec3(0.0f) }), mMode(mode), mVisible(false)
{
	mGizmoPass = std::make_unique<GizmoPass>();
	mGizmoTechnique = std::make_unique<Technique>(mGizmoPass.get());

	mRotationMesh = loadRotationGizmo();
	initSceneNode(mRotationGizmoNode, mRotationMesh, "Rotation Gizmo");

	mScaleMesh = loadScaleGizmo();
	initSceneNode(mScaleGizmoNode, mScaleMesh, "Scale Gizmo");
	
	mTranslationMesh = loadTranslationGizmo();
	initSceneNode(mTranslationGizmoNode, mTranslationMesh, "Translation Gizmo");

	setMode(Mode::ROTATE);
}

nex::gui::Gizmo::~Gizmo() = default;

void nex::gui::Gizmo::update(const nex::Camera& camera)
{
	const auto distance = length(mActiveGizmoNode->getPosition() - camera.getPosition());

	if (distance > 0.0001)
	{
		const float w = (camera.getProjectionMatrix() * camera.getView() * glm::vec4(mActiveGizmoNode->getPosition(), 1.0)).w;
		mActiveGizmoNode->setScale(glm::vec3(w) / 8.0f);
		mActiveGizmoNode->updateWorldTrafoHierarchy(true);
	}
}

void nex::gui::Gizmo::activate(const Ray& screenRayWorld, const float cameraViewFieldRange)
{
	isHovering(screenRayWorld, cameraViewFieldRange, &mActivationState);

	highlightAxis(mActivationState.axis);
	mLastFrameMultiplier = 0.0f;
	mRotationAcc = 0.0f;
}

nex::gui::Gizmo::Mode nex::gui::Gizmo::getMode() const
{
	return mMode;
}

const nex::gui::Gizmo::Active& nex::gui::Gizmo::getState() const
{
	return mActivationState;
}

void nex::gui::Gizmo::highlightAxis(Axis axis)
{
	mGizmoPass->bind();
	mGizmoPass->setSelectedAxis(axis);
}

bool nex::gui::Gizmo::isHovering(const Ray& screenRayWorld, const float cameraViewFieldRange, Active* active) const
{
	if (mMode == Mode::ROTATE)
	{
		return isHoveringRotate(screenRayWorld, cameraViewFieldRange, active);
	}


	const auto& position = mActiveGizmoNode->getPosition();
	const Ray xAxis(position, { 1.0f, 0.0f, 0.0f });
	const Ray yAxis(position, { 0.0f, 1.0f, 0.0f });
	const Ray zAxis(position, { 0.0f, 0.0f, getZValue(1.0f) });

	const Data xTest = { xAxis.calcClosestDistance(screenRayWorld), xAxis.getDir(), Axis::X };
	const Data yTest = { yAxis.calcClosestDistance(screenRayWorld), yAxis.getDir(), Axis::Y };
	const Data zTest = { zAxis.calcClosestDistance(screenRayWorld), zAxis.getDir(), Axis::Z };

	const auto* nearest = &xTest;

	if (compare(*nearest, yTest) > 0) {
		nearest = &yTest;
	}

	if (compare(*nearest, zTest) > 0) {
		nearest = &zTest;
	}

	const float distanceToCamera = length(position - screenRayWorld.getOrigin());
	const float range = std::clamp(distanceToCamera / cameraViewFieldRange, 0.0001f, 0.5f);

	const auto& scale = mActiveGizmoNode->getScale();
	const bool selected = (nearest->result.distance <= range)
		&& isInRange(nearest->result.multiplier, 0.0f, scale[(unsigned)nearest->axis]);


	fillActivationState(active, selected, nearest->axis, screenRayWorld.getPoint(nearest->result.otherMultiplier));

	return selected;
}

bool nex::gui::Gizmo::isVisible() const
{
	return mVisible;
}

nex::SceneNode* nex::gui::Gizmo::getGizmoNode()
{
	return mActiveGizmoNode;
}

void nex::gui::Gizmo::transform(const Ray& screenRayWorld, SceneNode& node, const MouseOffset& frameData)
{
	if (!mActivationState.isActive) return;

	//TODO Rotation gizmo needs to be handled differently

	if (mMode == Mode::ROTATE)
	{
		transformRotate(screenRayWorld, node);
	} else
	{
		const auto& position = mActivationState.originalPosition;

		Ray axis(position, { 1.0f, 0.0f, 0.0f });

		if (mActivationState.axis == Axis::Y)
			axis = { position, { 0.0f, 1.0f, 0.0f } };
		if (mActivationState.axis == Axis::Z)
			axis = { position, { 0.0f, 0.0f, getZValue(1.0f) } };

		const auto test = axis.calcClosestDistance(screenRayWorld);
		const auto frameDiff = test.multiplier - mLastFrameMultiplier;
		mLastFrameMultiplier = test.multiplier;

		if (test.parallel) return;

		if (mMode == Mode::SCALE)
		{
			auto scale = maxVec(node.getScale() + frameDiff * axis.getDir(), glm::vec3(0.0f));
			node.setScale(scale);

		}
		else if (mMode == Mode::TRANSLATE)
		{
			node.setPosition(node.getPosition() + frameDiff * axis.getDir());
			mActiveGizmoNode->setPosition(node.getPosition());
		}
	}


	node.updateWorldTrafoHierarchy(true);
	mActiveGizmoNode->updateWorldTrafoHierarchy(true);
}

void nex::gui::Gizmo::deactivate()
{
	mActivationState = { false, Axis::INVALID };
	highlightAxis(mActivationState.axis);
}

void nex::gui::Gizmo::setMode(Mode mode)
{
	mMode = mode;

	switch(mode)
	{
	case Mode::ROTATE:
		mActiveGizmoNode = mRotationGizmoNode;
		break;
	case Mode::SCALE:
		mActiveGizmoNode = mScaleGizmoNode;
		break;
	case Mode::TRANSLATE:
		mActiveGizmoNode = mTranslationGizmoNode;
		break;
	}

	deactivate();
}

void nex::gui::Gizmo::show(Scene& scene, const SceneNode& node)
{
	scene.addRoot(mActiveGizmoNode);
	mActiveGizmoNode->setPosition(node.getPosition());
	mActiveGizmoNode->updateWorldTrafoHierarchy(true);
	mVisible = true;
}

void nex::gui::Gizmo::hide(Scene& scene)
{
	scene.removeRoot(mActiveGizmoNode);
	mVisible = false;
}

int nex::gui::Gizmo::compare(const Data& first, const Data& second) const
{
	const auto& scale = mActiveGizmoNode->getScale();

	const auto scaleFirst = scale[(unsigned)first.axis];
	const auto scaleSecond = scale[(unsigned)second.axis];

	if (!isInRange(first.result.multiplier, 0.0f, scaleFirst))
		return 1;
	if (!isInRange(second.result.multiplier, 0.0f, scaleSecond))
		return -1;

	if (first.result.distance < second.result.distance)
		return -1;
	else if (first.result.distance == second.result.distance)
		return 0;
	return 1;
}

void nex::gui::Gizmo::initSceneNode(SceneNode*& node, StaticMeshContainer* container, const char* debugName)
{
	node = container->createNodeHierarchy(mNodeGeneratorScene.get());
	node->setSelectable(false);
	node->updateWorldTrafoHierarchy(true);
	node->mDebugName = debugName;
}

bool nex::gui::Gizmo::isHoveringRotate(const Ray& screenRayWorld, const float cameraViewFieldRange,
	Active* active) const
{

	const auto& origin = mActiveGizmoNode->getPosition();

	const auto yzPlaneIntersection = screenRayWorld.intersects({ {1,0,0}, origin});
	const auto xzPlaneIntersection = screenRayWorld.intersects({ {0,1,0}, origin });
	const auto xyPlaneIntersection = screenRayWorld.intersects({ {0,0,getZValue(1.0f)}, origin });

	const float distanceToCamera = length(origin - screenRayWorld.getOrigin());
	const float range = std::clamp(distanceToCamera / cameraViewFieldRange, 0.0001f, 0.5f);
	
	// Note: we assume a uniform scale for the gizmo!
	const float maxRange = mActiveGizmoNode->getScale().x + range;
	const float minRange = mActiveGizmoNode->getScale().x - range;


	bool selected = true;
	Axis axis = Axis::INVALID;
	float multiplier = 0.0f;

	if (checkNearPlaneCircle(yzPlaneIntersection, screenRayWorld, origin, minRange, maxRange, multiplier))
	{
		axis = Axis::X;
		std::cout << "Is in range for x rotation!" << std::endl;

	} else if (checkNearPlaneCircle(xzPlaneIntersection, screenRayWorld, origin, minRange, maxRange, multiplier))
	{
		axis = Axis::Y;
		std::cout << "Is in range for y rotation!" << std::endl;

	} else if (checkNearPlaneCircle(xyPlaneIntersection, screenRayWorld, origin, minRange, maxRange, multiplier))
	{
		axis = Axis::Z;
		std::cout << "Is in range for z rotation!" << std::endl;
	} else
	{
		selected = false;
	}

	fillActivationState(active, selected, axis, screenRayWorld.getPoint(multiplier));

	return selected;
}

bool nex::gui::Gizmo::checkNearPlaneCircle(const Ray::RayPlaneIntersection& testResult, const Ray& ray,
	const glm::vec3& circleOrigin, float minRadius, float maxRadius, float& multiplierOut) const
{
	if (!testResult.intersected) return false;


	if (testResult.intersected && !testResult.parallel)
	{
		multiplierOut = testResult.multiplier;
		const auto pointOnPlane = ray.getPoint(testResult.multiplier);
		const auto distance = length(circleOrigin - pointOnPlane);
		return isInRange(distance, minRadius, maxRadius);
	}

	// ray is parallel to plane, this means min radius doesn't matter
	// We just check the distance from the circle origin to the ray
	const auto pointDistance = ray.calcClosestDistance(circleOrigin);
	multiplierOut = pointDistance.multiplier;
	return pointDistance.distance <= maxRadius;

}

void nex::gui::Gizmo::fillActivationState(Active* active, bool isActive, Axis axis, const glm::vec3 position)
{
	if (!active) return;

	active->isActive = isActive;
	if (active->isActive) {
		active->axis = axis;

		if (active->axis == Axis::X)
			active->axisVec = { 1.0f, 0.0f, 0.0f };
		if (active->axis == Axis::Y)
			active->axisVec = { 0.0f, 1.0f, 0.0f };
		if (active->axis == Axis::Z)
			active->axisVec = { 0.0f, 0.0f, getZValue(1.0f) };

		active->originalPosition = position;
	}
	else {
		active->axis = Axis::INVALID;
		active->originalPosition = glm::vec3(0.0f);
	}
}

void nex::gui::Gizmo::transformRotate(const Ray& ray, SceneNode& node)
{
	const auto& origin = mActiveGizmoNode->getPosition();
	const auto planeIntersection = ray.intersects({ mActivationState.axisVec, origin });

	if (!planeIntersection.intersected) return;

	glm::vec3 newPosition;

	if (planeIntersection.parallel)
	{
		Ray axisRay(origin, mActivationState.axisVec);
		const auto distanceToAxis = ray.calcClosestDistance(axisRay);
		const auto& radius = mActiveGizmoNode->getScale().x;
		const auto multiplier = length(origin - ray.getOrigin()) - radius;

		//TODO
		return;

	} else
	{
		newPosition = ray.getPoint(planeIntersection.multiplier);
	}

	const auto vec1 = normalize(origin - mActivationState.originalPosition);
	const auto vec2 = normalize(origin - newPosition);

	auto angle = acos(dot(vec1, vec2));

	std::cout << "vec1 = " << vec1 << ", vec2 = " << vec2 << std::endl;
	std::cout << "  original position = " << mActivationState.originalPosition << ", new position = " << newPosition << std::endl;


	auto zAxis = glm::vec3(0, 0, getZValue(1.0f));
	auto xAxis = glm::vec3(1.0f, 0, 0);
	auto d = dot(xAxis, vec1) - dot(zAxis, vec1);
	d -= dot(xAxis, vec2) - dot(zAxis, vec2);

	if (d < 0)
	{
		angle *= -1;
	}

	std::cout << "     angle = " << angle << ", dot = " << d << std::endl;

	if (!std::isnan(angle))
	{
		auto diff = angle - mRotationAcc;
		mRotationAcc = angle;
		node.rotateGlobal(mActivationState.axisVec, diff);
	}
}

class MaterialLoader : public nex::DefaultMaterialLoader
{
public:
	MaterialLoader(nex::Technique* technique) : DefaultMaterialLoader(), mTechnique(technique) {};

	virtual void loadShadingMaterial(const aiScene* scene, nex::MaterialStore& store, unsigned materialIndex) const override
	{
		aiColor3D color;
		if (AI_SUCCESS == scene->mMaterials[materialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color) )
		{
			store.diffuseColor = glm::vec4(color.r, color.g, color.b, 1.0f);
		}
	}

	std::unique_ptr<nex::Material> createMaterial(const nex::MaterialStore& store) const override
	{
		auto material = std::make_unique<nex::Material>(mTechnique);

		auto& state = material->getRenderState();
		state.doCullFaces = false;
		state.doShadowCast = false;
		state.doShadowReceive = false;
		state.doBlend = false;
		state.fillMode = nex::FillMode::FILL;
		state.doDepthTest = false;
		state.doDepthWrite = false;

		auto* pass = mTechnique->getActiveSubMeshPass();
		material->set(pass->getShader()->getUniformLocation("axisColor"), glm::vec3(store.diffuseColor));

		return material;
	}

private:
	nex::Technique* mTechnique;
};

nex::StaticMeshContainer* nex::gui::Gizmo::loadRotationGizmo()
{
	return StaticMeshManager::get()->loadModel(
		"_intern/gizmo/rotation-gizmo.obj",
		MeshLoader<VertexPosition>(),
		MaterialLoader(mGizmoTechnique.get()));
}

nex::StaticMeshContainer* nex::gui::Gizmo::loadTranslationGizmo()
{
	return StaticMeshManager::get()->loadModel(
		"_intern/gizmo/translation-gizmo.obj",
		MeshLoader<VertexPosition>(),
		MaterialLoader(mGizmoTechnique.get()));
}

nex::StaticMeshContainer* nex::gui::Gizmo::loadScaleGizmo()
{
	return StaticMeshManager::get()->loadModel(
		"_intern/gizmo/scale-gizmo.obj",
		MeshLoader<VertexPosition>(),
		MaterialLoader(mGizmoTechnique.get()));
}