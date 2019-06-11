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
							mActivationState({ false, Axis::INVALID, glm::vec3(0.0f) }), mMode(mode)
{
	mGizmoPass = std::make_unique<GizmoPass>();
	mGizmoTechnique = std::make_unique<Technique>(mGizmoPass.get());

	mScaleMesh = loadScaleGizmo();
	initSceneNode(mScaleGizmoNode, mScaleMesh, "Scale Gizmo");
	
	mTranslationMesh = loadTranslationGizmo();
	initSceneNode(mTranslationGizmoNode, mTranslationMesh, "Translation Gizmo");

	setMode(Mode::SCALE);
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
	//TODO Rotation gizmo must be handled differently
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

	if (active)
	{
		active->isActive = selected;
		if (active->isActive) {
			active->axis = nearest->axis;
			active->originalPosition = mActiveGizmoNode->getPosition(); 

			//if (mMode ==Mode::TRANSLATE)
				active->originalPosition += nearest->result.multiplier * nearest->axisVector;
		}
		else {
			active->axis = Axis::INVALID;
			active->originalPosition = glm::vec3(0.0f);
		}
	}

	return selected;
}

nex::SceneNode* nex::gui::Gizmo::getGizmoNode()
{
	return mActiveGizmoNode;
}

void nex::gui::Gizmo::transform(const Ray& screenRayWorld, SceneNode& node, const MouseOffset& frameData)
{
	if (!mActivationState.isActive) return;

	//TODO Rotation gizmo needs to be handled differently

	const auto& position = mActivationState.originalPosition;

	Ray axis(position, { 1.0f, 0.0f, 0.0f });

	if (mActivationState.axis == Axis::Y)
		axis = { position, { 0.0f, 1.0f, 0.0f } };
	if (mActivationState.axis == Axis::Z)
		axis = { position, { 0.0f, 0.0f, getZValue(1.0f) } };

	const auto test = axis.calcClosestDistance(screenRayWorld);

	if (test.parallel) return;

	const auto frameDiff = test.multiplier - mLastFrameMultiplier;
	mLastFrameMultiplier = test.multiplier;

	if (mMode == Mode::SCALE)
	{
		auto scale = maxVec(node.getScale() + frameDiff * axis.getDir(), glm::vec3(0.0f));
		node.setScale(scale);
		
	} else if (mMode == Mode::TRANSLATE)
	{
		node.setPosition(node.getPosition() + frameDiff * axis.getDir());
		mActiveGizmoNode->setPosition(node.getPosition());
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
}

void nex::gui::Gizmo::hide(Scene& scene)
{
	scene.removeRoot(mActiveGizmoNode);
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