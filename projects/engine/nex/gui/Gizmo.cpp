#include <nex/gui/Gizmo.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/shader/ShaderProvider.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/mesh/Mesh.hpp>
#include "nex/math/Ray.hpp"
#include <nex/math/Math.hpp>
#include <nex/mesh/MeshManager.hpp>
#include "nex/camera/Camera.hpp"
#include <nex/math/Math.hpp>
#include <nex/math/Constant.hpp>
#include "nex/math/Torus.hpp"
#include "nex/math/Sphere.hpp"
#include "nex/math/Circle.hpp"

class nex::gui::Gizmo::Material : public nex::Material {
public:
	Material(Gizmo::GizmoPass* shader) : nex::Material(std::make_shared<ShaderProvider>((Shader*)shader)) {
	}

	glm::vec3 axisColor;
};

class nex::gui::Gizmo::GizmoPass : public TransformShader
{
public:
	GizmoPass() : TransformShader(ShaderProgram::create("gui/gizmo/gizmo_vs.glsl", "gui/gizmo/gizmo_fs.glsl"))
	{
		bind();
		mSelectedAxis = { mProgram->getUniformLocation("selectedAxis"), UniformType::UINT };
		mAxisColor = { mProgram->getUniformLocation("axisColor"), UniformType::VEC3 };

		// Default state: No axis is selected
		setSelectedAxis(Axis::INVALID);
	}

	void setSelectedAxis(Axis axis)
	{
		mProgram->setUInt(mSelectedAxis.location, (unsigned)axis);
	}

	void setAxisColor(const glm::vec3& color) {
		mProgram->setVec3(mAxisColor.location, color);
	}

	void updateMaterial(const nex::Material& m) override 
	{
		const auto& material = (const nex::gui::Gizmo::Material&)m;
		setAxisColor(material.axisColor);
	}

private:
	Uniform mSelectedAxis;
	Uniform mAxisColor;
};

float nex::gui::Gizmo::Active::calcRange(const Ray& ray, const glm::vec3& position, const Camera& camera)
{
	const auto distanceToCamera = length(position - ray.getOrigin());
	return std::clamp(distanceToCamera / (camera.getFarDistance() - camera.getNearDistance()), 0.0001f, 0.5f);
}

nex::gui::Gizmo::Gizmo(Mode mode) : mTranslationGizmoNode(nullptr),
mActivationState({}), mMode(mode), mVisible(false)
{
	mGizmoPass = std::make_unique<Gizmo::GizmoPass>();
	mMaterialLoader = std::make_unique<MaterialLoader>(mGizmoPass.get());
	mMeshLoader = std::make_unique<MeshLoader<VertexPosition>>();

	mRotationMesh = loadRotationGizmo();
	mRotationMesh->finalize();
	initSceneNode(mRotationGizmoNode, mRotationMesh.get(), "Rotation Gizmo");

	mScaleMesh = loadScaleGizmo();
	mScaleMesh->finalize();
	initSceneNode(mScaleGizmoNode, mScaleMesh.get(), "Scale Gizmo");

	mTranslationMesh = loadTranslationGizmo();
	mTranslationMesh->finalize();
	initSceneNode(mTranslationGizmoNode, mTranslationMesh.get(), "Translation Gizmo");

	setMode(Mode::ROTATE);
}

nex::gui::Gizmo::~Gizmo() = default;

void nex::gui::Gizmo::syncTransformation()
{
	if (mModifiedNode)
	{
		mModifiedNode->updateTrafo(true);
		mActiveGizmoVob->setPositionWorld(mModifiedNode->getPositionWorld());
		mActiveGizmoVob->updateTrafo(true);
	}
}

void nex::gui::Gizmo::update(const nex::Camera& camera, Vob* vob)
{
	mModifiedNode = vob;

	syncTransformation();

	const auto distance = length(mActiveGizmoVob->getPositionWorld() - camera.getPosition());

	if (distance > 0.0001)
	{
		const float w = (camera.getProjectionMatrix() * camera.getView() * glm::vec4(mActiveGizmoVob->getPositionWorld(), 1.0)).w;
		mActiveGizmoVob->setScaleLocal(glm::vec3(w) / 8.0f);
	}



	mActiveGizmoVob->updateTrafo(true);

}

void nex::gui::Gizmo::activate(const Ray& screenRayWorld, const Camera& camera)
{
	isHovering(screenRayWorld, camera, true);

	highlightAxis(mActivationState.axis);
	mLastFrameMultiplier = 0.0f;
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

bool nex::gui::Gizmo::isHovering(const Ray& screenRayWorld, const Camera& camera)
{
	return isHovering(screenRayWorld, camera, false);
}

bool nex::gui::Gizmo::isVisible() const
{
	return mVisible;
}

nex::Vob* nex::gui::Gizmo::getGizmoNode()
{
	return mActiveGizmoVob;
}

void nex::gui::Gizmo::transform(const Ray& screenRayWorld, const Camera& camera, const MouseOffset& frameData)
{
	if (!mActivationState.isActive) return;

	if (mMode == Mode::ROTATE)
	{
		transformRotate(screenRayWorld, camera);
	}
	else
	{
		const auto& position = mActivationState.originalPosition;

		Ray axis(position, { 1.0f, 0.0f, 0.0f });

		if (mActivationState.axis == Axis::Y)
			axis = { position, { 0.0f, 1.0f, 0.0f } };
		if (mActivationState.axis == Axis::Z)
			axis = { position, { 0.0f, 0.0f, getZValue(1.0f) } };

		const auto test = axis.calcClosestDistance(screenRayWorld);
		const float frameDiff = test.multiplier - mLastFrameMultiplier;
		mLastFrameMultiplier = test.multiplier;

		if (test.parallel) return;

		if (mMode == Mode::SCALE)
		{
			//auto scale = maxVec(mModifiedNode->getScaleLocal() + frameDiff * axis.getDir(), glm::vec3(0.0f));
			//mModifiedNode->setScaleLocal(scale);

			auto scaleAxis = glm::vec3(1.0f) + frameDiff * axis.getDir();

			auto scale = maxVec(scaleAxis, glm::vec3(0.0f));
			mModifiedNode->setScaleWorld(scale);
			//mModifiedNode->setScaleLocal(scale);

		}
		else if (mMode == Mode::TRANSLATE)
		{
			mModifiedNode->setPositionWorld(mModifiedNode->getPositionWorld() + frameDiff * axis.getDir());
			mActiveGizmoVob->setPositionWorld(mModifiedNode->getPositionWorld());
		}
	}


	mModifiedNode->updateTrafo(true);
	mActiveGizmoVob->updateTrafo(true);
}

void nex::gui::Gizmo::deactivate()
{
	mActivationState = {};
	highlightAxis(mActivationState.axis);
}

void nex::gui::Gizmo::setMode(Mode mode)
{
	mMode = mode;

	if (mVisible) {
		mScene->acquireLock();
		mScene->removeActiveVobUnsafe(mActiveGizmoVob);
	}

	switch (mode)
	{
	case Mode::ROTATE:
		mActiveGizmoVob = mRotationGizmoNode.get();
		break;
	case Mode::SCALE:
		mActiveGizmoVob = mScaleGizmoNode.get();
		break;
	case Mode::TRANSLATE:
		mActiveGizmoVob = mTranslationGizmoNode.get();
		break;
	}

	if (mVisible) {
		mScene->acquireLock();
		mScene->addActiveVobUnsafe(mActiveGizmoVob);
	}
	syncTransformation();
	deactivate();
}

void nex::gui::Gizmo::show(Scene* scene)
{
	mScene = scene;
	mScene->acquireLock();
	mScene->addActiveVobUnsafe(mActiveGizmoVob);
	//mActiveGizmoVob->setPosition(node->getPosition());
	//mActiveGizmoVob->updateTrafo(true);
	//mModifiedNode = node;
	mVisible = true;
}

void nex::gui::Gizmo::hide()
{
	if (mScene != nullptr) {
		mScene->acquireLock();
		mScene->removeActiveVobUnsafe(mActiveGizmoVob);
	}
	mVisible = false;
	mModifiedNode = nullptr;
	mScene = nullptr;
}

int nex::gui::Gizmo::compare(const Data& first, const Data& second) const
{
	const auto& scale = mActiveGizmoVob->getScaleLocal();

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

float nex::gui::Gizmo::calcRotation(const Ray& ray, const glm::vec3& axis, const glm::vec3& orthoAxis,
	const Camera& camera) const
{
	const auto& origin = mActiveGizmoVob->getPositionWorld();

	const auto radius = mActiveGizmoVob->getScaleLocal().x;
	const Sphere sphere(origin, radius);
	const Ray tRay(ray.getOrigin(), origin - ray.getOrigin());
	const auto sphereInt = sphere.intersects(tRay);
	const auto angleToAxis = dot(axis, normalize(origin - camera.getPosition()));
	glm::vec3 projectedPoint = origin;

	if (abs(angleToAxis) > 0.06f)
	{
		Plane plane = { axis, origin };
		const auto planeIntersection = plane.intersects(ray);

		if (!planeIntersection.parallel)
		{
			const auto newPoint = ray.getPoint(planeIntersection.multiplier);
			if (glm::length2(newPoint - origin) > 0.01)
			{
				projectedPoint = newPoint;
			}
		}
	}
	else
	{
		//sphereInt.intersectionCount != 0 &&
		const auto closestPoint = ray.getPoint(sphereInt.firstMultiplier);
		Circle3D circle(origin, axis, radius);

		if (!circle.project(closestPoint, projectedPoint))
		{
			projectedPoint = origin;
		}
	}

	const auto vec2 = normalize(projectedPoint - origin);
	const auto d = dot(orthoAxis, vec2);
	const auto angle = acos(d);

	const auto test = dot(normalize(cross(orthoAxis, vec2)), normalize(axis));
	return angle * (test / abs(test));
}

void nex::gui::Gizmo::initSceneNode(std::unique_ptr<Vob>& vob, MeshGroup* container, const char* debugName)
{
	vob = std::make_unique<Vob>(nullptr);
	vob->setBatches(container->getBatches());
	vob->getName() = debugName;
	vob->setSelectable(false);
	vob->updateTrafo(true);
}

bool nex::gui::Gizmo::isHovering(const Ray& screenRayWorld, const Camera& camera, bool fillActive)
{
	if (mMode == Mode::ROTATE)
	{
		return isHoveringRotate(screenRayWorld, camera, fillActive);
	}


	const auto& position = mActiveGizmoVob->getPositionWorld();
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

	const auto& scale = mActiveGizmoVob->getScaleLocal();
	const bool selected = (nearest->result.distance <= Active::calcRange(screenRayWorld, position, camera))
		&& isInRange((float)nearest->result.multiplier, 0.0f, scale[(unsigned)nearest->axis]);


	if (fillActive)
		fillActivationState(mActivationState, selected, nearest->axis, screenRayWorld.getPoint(nearest->result.otherMultiplier), screenRayWorld, camera);

	return selected;
}

bool nex::gui::Gizmo::isHoveringRotate(const Ray& screenRayWorld, const Camera& camera,
	bool fillActive)
{
	const auto& origin = mActiveGizmoVob->getPositionWorld();

	constexpr glm::vec3 xAxis(1, 0, 0);
	constexpr glm::vec3 yAxis(0, 1, 0);
	const glm::vec3 zAxis(0, 0, getZValue(1.0f));

	const float range = Active::calcRange(screenRayWorld, origin, camera);

	bool selected = true;
	Axis axis = Axis::INVALID;
	Torus::RayIntersection intersection;
	Torus torus(mActiveGizmoVob->getScaleLocal().x, range);


	if (hitsTorus(torus, xAxis, origin, screenRayWorld, intersection))
	{
		axis = Axis::X;
		std::cout << "Is in range for x rotation!" << std::endl;

	}
	else if (hitsTorus(torus, yAxis, origin, screenRayWorld, intersection))
	{
		axis = Axis::Y;
		std::cout << "Is in range for y rotation!" << std::endl;

	}
	else if (hitsTorus(torus, zAxis, origin, screenRayWorld, intersection))
	{
		axis = Axis::Z;
		std::cout << "Is in range for z rotation!" << std::endl;

	}
	else
	{
		selected = false;
	}

	const float closestDistanceMultiplier = screenRayWorld.calcClosestDistance(origin).multiplier;
	const auto closestPoint = screenRayWorld.getPoint(closestDistanceMultiplier);
	Circle3D circle(origin, { 1,0,0 }, mActiveGizmoVob->getScaleLocal().x);

	glm::vec3 projectedPoint;
	if (!circle.project(closestPoint, projectedPoint)) //TODO
	{
		projectedPoint = origin;
	}

	if (fillActive)
	{
		fillActivationState(mActivationState, selected, axis, projectedPoint, screenRayWorld, camera);
	}

	return selected;
}

bool nex::gui::Gizmo::checkNearPlaneCircle(const Plane::RayIntersection& testResult, const Ray& ray,
	const glm::vec3& circleOrigin, float minRadius, float maxRadius, float& multiplierOut) const
{
	if (!testResult.intersected) return false;


	if (testResult.intersected && !testResult.parallel)
	{
		multiplierOut = testResult.multiplier;
		const auto pointOnPlane = ray.getPoint(testResult.multiplier);
		const auto radius = mActiveGizmoVob->getScaleLocal().x;
		const auto point = circleOrigin + radius * normalize(pointOnPlane - circleOrigin);
		const auto diff = length(pointOnPlane - point);//length(circleOrigin - pointOnPlane);
		return diff < (maxRadius - minRadius);
		//return isInRange(distance, minRadius, maxRadius);
	}

	// ray is parallel to plane, this means min radius doesn't matter
	// We just check the distance from the circle origin to the ray
	const auto pointDistance = ray.calcClosestDistance({ {circleOrigin}, {0,1,0} });
	//const auto pointDistance = ray.calcClosestDistance(circleOrigin);
	multiplierOut = pointDistance.multiplier;
	const auto radius = mActiveGizmoVob->getScaleLocal().x;
	return pointDistance.distance <= (0.1);

}

bool nex::gui::Gizmo::hitsTorus(const Torus& torus, const glm::vec3& orientation, const glm::vec3& origin,
	const Ray& ray, Torus::RayIntersection& intersectionTest)
{
	// transform the ray to torus space. 
	// Note that the torus has its origin at (0,0,0) and its up-vector is (0,1,0) 
	const auto rotation = nex::rotate(glm::vec3(0.0f, 1.0f, 0.0f), orientation);
	const auto newOrigin = rotation * (ray.getOrigin() - origin);
	const auto newDir = rotation * ray.getDir();
	const Ray transformedRay(newOrigin, newDir);

	intersectionTest = torus.intersects(transformedRay);

	return intersectionTest.intersectionCount != 0;
}

void nex::gui::Gizmo::fillActivationState(Active& active,
	bool isActive,
	Axis axis,
	const glm::vec3& position,
	const Ray& ray,
	const Camera& camera) const
{
	active.isActive = isActive;

	if (!active.isActive) {

		active.axis = Axis::INVALID;
		active.originalPosition = glm::vec3(0.0f);
		return;
	}

	active.axis = axis;

	if (active.axis == Axis::X)
	{
		active.axisVec = { 1.0f, 0.0f, 0.0f };
		active.orthoAxisVec = { 0,1,0 };
	}
	if (active.axis == Axis::Y)
	{
		active.axisVec = { 0.0f, 1.0f, 0.0f };
		active.orthoAxisVec = { 1,0,0 };
	}
	if (active.axis == Axis::Z)
	{
		active.axisVec = { 0.0f, 0.0f, getZValue(1.0f) };
		active.orthoAxisVec = { 0,1,0 };
	}

	active.originalPosition = position;

	if (mModifiedNode)
	{
		active.originalRotation = mModifiedNode->getRotationLocal();
		active.startRotationAngle = calcRotation(ray, mActivationState.axisVec, mActivationState.orthoAxisVec, camera);
		active.range = Active::calcRange(ray, mActiveGizmoVob->getPositionWorld(), camera);
	}
}

void nex::gui::Gizmo::transformRotate(const Ray& ray, const Camera& camera)
{
	std::cout << "direction = " << ray.getDir() << std::endl;
	const auto angle = calcRotation(ray, mActivationState.axisVec, mActivationState.orthoAxisVec, camera);
	std::cout << "angle = " << glm::degrees(angle) << std::endl;

	if (isValid(angle))
	{
		const auto rotationAdd = glm::rotate(glm::quat(1, 0, 0, 0), angle - mActivationState.startRotationAngle, mActivationState.axisVec);
		mModifiedNode->setRotationLocal(rotationAdd * mActivationState.originalRotation);
	}
}


class nex::gui::Gizmo::MaterialLoader : public nex::DefaultMaterialLoader
{
public:
	MaterialLoader(Gizmo::GizmoPass* shader) : DefaultMaterialLoader(), mShader(shader) {};

	virtual void loadShadingMaterial(const std::filesystem::path& meshPathAbsolute, const aiScene* scene, nex::MaterialStore& store, unsigned materialIndex) const override
	{
		aiColor3D color;
		if (AI_SUCCESS == scene->mMaterials[materialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color))
		{
			store.diffuseColor = glm::vec4(color.r, color.g, color.b, 1.0f);
		}
	}

	std::unique_ptr<nex::Material> createMaterial(const nex::MaterialStore& store) const override
	{
		auto material = std::make_unique<Gizmo::Material>(mShader);

		auto& state = material->getRenderState();
		state.doCullFaces = false;
		state.doShadowCast = false;
		state.doShadowReceive = false;
		state.doBlend = false;
		state.fillMode = nex::FillMode::FILL;
		state.doDepthTest = false;
		state.doDepthWrite = false;
		state.isTool = true;



		material->axisColor = glm::vec3(store.diffuseColor);

		return material;
	}

private:
	nex::gui::Gizmo::GizmoPass* mShader;
};

std::unique_ptr<nex::MeshGroup> nex::gui::Gizmo::loadRotationGizmo()
{
	auto meshGroup = MeshManager::get()->loadModel(
		"_intern/gizmo/rotation-gizmo.obj",
		*mMaterialLoader,
		mMeshLoader.get());
	meshGroup->finalize();
	return meshGroup;
}

std::unique_ptr<nex::MeshGroup> nex::gui::Gizmo::loadTranslationGizmo()
{
	auto meshGroup = MeshManager::get()->loadModel(
		"_intern/gizmo/translation-gizmo.obj",
		*mMaterialLoader,
		mMeshLoader.get());
	meshGroup->finalize();
	return meshGroup;
}

std::unique_ptr<nex::MeshGroup> nex::gui::Gizmo::loadScaleGizmo()
{
	auto meshGroup = MeshManager::get()->loadModel(
		"_intern/gizmo/scale-gizmo.obj",
		*mMaterialLoader,
		mMeshLoader.get());
	meshGroup->finalize();
	return meshGroup;
}