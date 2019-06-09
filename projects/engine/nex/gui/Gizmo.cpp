#include <nex/gui/Gizmo.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/shader/Technique.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/Scene.hpp>
#include <nex/mesh/MeshFactory.hpp>
#include <nex/mesh/Mesh.hpp>
#include "nex/math/Ray.hpp"
#include <nex/math/Math.hpp>

class nex::gui::Gizmo::TranslationGizmoPass : public TransformPass
{
public:
	TranslationGizmoPass() : TransformPass(Shader::create("gui/gizmo/gizmo_vs.glsl", "gui/gizmo/gizmo_fs.glsl"))
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

nex::gui::Gizmo::Gizmo() : mNodeGeneratorScene(std::make_unique<Scene>()), mTranslationGizmoNode(nullptr),
							mActivationState({ false, Axis::INVALID })
{
	mTranslationGizmoPass = std::make_unique<TranslationGizmoPass>();
	mGizmoTechnique = std::make_unique<Technique>(mTranslationGizmoPass.get());
	auto material = std::make_unique<Material>(mGizmoTechnique.get());

	auto& state = material->getRenderState();
	state.doCullFaces = false;
	state.doShadowCast = false;
	state.doShadowReceive = false;
	state.doBlend = false;
	state.fillMode = FillMode::LINE;
	state.doDepthTest = false;
	state.doDepthWrite = false;

	mTranslationMesh = std::make_unique<StaticMeshContainer>();
	mTranslationMesh->add(std::move(createTranslationMesh()), std::move(material));
	mTranslationGizmoNode = mTranslationMesh->createNodeHierarchy(mNodeGeneratorScene.get());
	mTranslationGizmoNode->setSelectable(false);

	mTranslationGizmoNode->setScale(glm::vec3(3.0f));
	mTranslationGizmoNode->updateWorldTrafoHierarchy();
}

nex::gui::Gizmo::~Gizmo() = default;

void nex::gui::Gizmo::activate(const Ray& screenRayWorld, const float cameraViewFieldRange)
{

	const auto& position = mTranslationGizmoNode->getPosition();
	const Ray xAxis(position, {1.0f, 0.0f, 0.0f});
	const Ray yAxis(position, { 0.0f, 1.0f, 0.0f });
	const Ray zAxis(position, { 0.0f, 0.0f, 1.0f });

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
	const float a = std::clamp(distanceToCamera / cameraViewFieldRange, 0.0001f, 0.5f);

	std::cout << "a = " << a << std::endl;

	const auto& scale = mTranslationGizmoNode->getScale();
	const bool selected = (nearest->result.distance <= a)  
					&& 	isInRange(nearest->result.multiplier, 0.0f, scale[(unsigned)nearest->axis]);

	
	std::cout << "nearest->result.multipler = " << nearest->result.multiplier 
	<< ", nearest->result.otherMultiplier = " << nearest->result.otherMultiplier 
	<< ", parallel = " << nearest->result.parallel << std::endl;


	mActivationState.isActive = selected;

	if (mActivationState.isActive) {
		mActivationState.axis = nearest->axis;
		mActivationState.originalPosition = mTranslationGizmoNode->getPosition() + nearest->result.multiplier * nearest->axisVector;
	}
	else {
		mActivationState.axis = Axis::INVALID;
		mActivationState.originalPosition = glm::vec3(0.0f);
	}


	std::cout << "Gizmo active = " << mActivationState.isActive << ", Axis = " << (unsigned)mActivationState.axis << std::endl;

	highlightAxis(mActivationState.axis);
	mLastFrameMultiplier = 0.0f;
}

const nex::gui::Gizmo::Active& nex::gui::Gizmo::getState() const
{
	return mActivationState;
}

void nex::gui::Gizmo::highlightAxis(Axis axis)
{
	mTranslationGizmoPass->bind();
	mTranslationGizmoPass->setSelectedAxis(axis);
}

nex::SceneNode* nex::gui::Gizmo::getGizmoNode()
{
	return mTranslationGizmoNode;
}

void nex::gui::Gizmo::transform(const Ray& screenRayWorld, SceneNode& node, const MouseOffset& frameData)
{
	if (!mActivationState.isActive) return;

	const auto& position = mActivationState.originalPosition;

	Ray axis(position, { 1.0f, 0.0f, 0.0f });

	if (mActivationState.axis == Axis::Y)
		axis = { position, { 0.0f, 1.0f, 0.0f } };
	if (mActivationState.axis == Axis::Z)
		axis = { position, { 0.0f, 0.0f, 1.0f } };

	const auto test = axis.calcClosestDistance(screenRayWorld);

	if (!test.parallel)
	{
		auto frameTranslationDiff = test.multiplier - mLastFrameMultiplier;
		mLastFrameMultiplier = test.multiplier;
		node.setPosition(node.getPosition() + frameTranslationDiff * axis.getDir());
		mTranslationGizmoNode->setPosition(node.getPosition());
		node.updateWorldTrafoHierarchy(true);
		mTranslationGizmoNode->updateWorldTrafoHierarchy(true);
	}
}

void nex::gui::Gizmo::deactivate()
{
	mActivationState = { false, Axis::INVALID };
	std::cout << "Gizmo active = " << mActivationState.isActive << ", Axis = " << (unsigned)mActivationState.axis << std::endl;
	highlightAxis(mActivationState.axis);
}

int nex::gui::Gizmo::compare(const Data& first, const Data& second) const
{
	const auto& scale = mTranslationGizmoNode->getScale();

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

std::unique_ptr<nex::Mesh> nex::gui::Gizmo::createTranslationMesh()
{
	//create vertices in CCW

	constexpr size_t vertexCount = 6;
	constexpr size_t indexCount = 6;

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	Vertex vertices[vertexCount];

	constexpr auto red = glm::vec3(1.0f, 0.0f, 0.0f);
	constexpr auto green = glm::vec3(0.0f, 1.0f, 0.0f);
	constexpr auto blue = glm::vec3(0.0f, 0.0f, 1.0f);


	vertices[0].position = glm::vec3(0.0f);
	vertices[0].color = red;
	vertices[1].position = glm::vec3(1.0f, 0.0f, 0.0f);
	vertices[1].color = red;
	
	vertices[2].position = glm::vec3(0.0f);
	vertices[2].color = green;
	vertices[3].position = glm::vec3(0.0f, 1.0f, 0.0f);
	vertices[3].color = green;

	vertices[4].position = glm::vec3(0.0f);
	vertices[4].color = blue;
	vertices[5].position = glm::vec3(0.0f, 0.0f, 1.0f);
	vertices[5].color = blue;

	unsigned indices[indexCount];

	indices[0] = 0;
	indices[1] = 1;

	indices[2] = 2;
	indices[3] = 3;

	indices[4] = 4;
	indices[5] = 5;


	VertexBuffer vertexBuffer;
	vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
	IndexBuffer indexBuffer(indices, indexCount, IndexElementType::BIT_32);

	VertexLayout layout;
	layout.push<glm::vec3>(1); // position
	layout.push<glm::vec3>(1); // color

	VertexArray vertexArray;
	vertexArray.bind();
	vertexArray.useBuffer(vertexBuffer, layout);

	vertexArray.unbind();
	indexBuffer.unbind();


	AABB boundingBox = { glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX) };
	auto mesh = std::make_unique<Mesh>(std::move(vertexArray), std::move(vertexBuffer), std::move(indexBuffer), std::move(boundingBox));

	mesh->setTopology(Topology::LINES);
	mesh->mDebugName = "Gizmo::Translation";
	return mesh;
}