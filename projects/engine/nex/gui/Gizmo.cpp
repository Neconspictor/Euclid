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

nex::gui::Gizmo::Gizmo() : mNodeGeneratorScene(std::make_unique<Scene>()), mTranslationGizmoNode(nullptr)
{
	mGizmoPass = std::make_unique<TransformPass>(Shader::create("gui/gizmo/gizmo_vs.glsl", "gui/gizmo/gizmo_fs.glsl"));
	mGizmoTechnique = std::make_unique<Technique>(mGizmoPass.get());
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
}

nex::gui::Gizmo::~Gizmo() = default;

nex::gui::Gizmo::Active nex::gui::Gizmo::isActive(const Ray& screenRayWorld, const float cameraViewFieldRange)
{

	const auto& position = mTranslationGizmoNode->getPosition();
	const Ray xAxis(position, {1.0f, 0.0f, 0.0f});
	const Ray yAxis(position, { 0.0f, 1.0f, 0.0f });
	const Ray zAxis(position, { 0.0f, 0.0f, 1.0f });

	const Data xTest = { xAxis.calcClosestDistance(screenRayWorld), Axis::X };
	const Data yTest = { yAxis.calcClosestDistance(screenRayWorld), Axis::Y };
	const Data zTest = { zAxis.calcClosestDistance(screenRayWorld), Axis::Z };

	const auto* nearest = &xTest;

	const auto& yResult = yTest.result;
	const auto& zResult = zTest.result;

	if (compare(*nearest, yTest) > 0) {
		nearest = &yTest;
	}

	if (compare(*nearest, zTest) > 0) {
		nearest = &zTest;
	}

	const float distanceToCamera = length(position - screenRayWorld.getOrigin());
	float a = std::clamp(distanceToCamera / cameraViewFieldRange, 0.0001f, 0.5f);

	std::cout << "a = " << a << std::endl;

	const auto& scale = mTranslationGizmoNode->getScale();
	bool selected = (nearest->result.distance <= a)  
					&& 	isInRange(nearest->result.multipler, 0.0f, scale[(unsigned)nearest->axis]);

	
	std::cout << "nearest->result.multipler = " << nearest->result.multipler 
	<< ", nearest->result.otherMultiplier = " << nearest->result.otherMultiplier 
	<< ", parallel = " << nearest->result.parallel << std::endl;

	return { selected, nearest->axis};
}

nex::SceneNode* nex::gui::Gizmo::getGizmoNode()
{
	return mTranslationGizmoNode;
}

int nex::gui::Gizmo::compare(const Data& first, const Data& second) const
{
	const auto& scale = mTranslationGizmoNode->getScale();

	const auto scaleFirst = scale[(unsigned)first.axis];
	const auto scaleSecond = scale[(unsigned)second.axis];

	if (!isInRange(first.result.multipler, 0.0f, scaleFirst))
		return 1;
	if (!isInRange(second.result.multipler, 0.0f, scaleSecond))
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