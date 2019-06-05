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

nex::gui::Picker::Picker() :
mBoundingBoxMesh(std::make_unique<StaticMeshContainer>()),
mLineMesh(std::make_unique<StaticMeshContainer>()), 
mSimpleColorPass(std::make_unique<SimpleColorPass>()),
mSimpleColorTechnique(std::make_unique<Technique>(mSimpleColorPass.get())),
mNodeGeneratorScene(std::make_unique<Scene>()),
mBoundingBoxNode(nullptr), mLineNode(nullptr)
{
	auto boxMaterial = std::make_unique<Material>(mSimpleColorTechnique.get());
	boxMaterial->getRenderState().fillMode = FillMode::LINE;
	boxMaterial->getRenderState().doCullFaces = false;
	boxMaterial->getRenderState().doShadowCast = false;


	auto lineMaterial = std::make_unique<Material>(mSimpleColorTechnique.get());
	lineMaterial->getRenderState().fillMode = FillMode::LINE;
	lineMaterial->getRenderState().doCullFaces = false;
	lineMaterial->getRenderState().doShadowCast = false;
	
	mBoundingBoxMesh->add(createBoundingBoxLineMesh(), std::move(boxMaterial));
	mBoundingBoxNode = mBoundingBoxMesh->createNodeHierarchy(mNodeGeneratorScene.get());
	mLineMesh->add(createLineMesh(), std::move(lineMaterial));
	mLineNode = mLineMesh->createNodeHierarchy(mNodeGeneratorScene.get());
}

nex::gui::Picker::~Picker() = default;


nex::SceneNode* nex::gui::Picker::pick(Scene& scene, const Ray& screenRayWorld)
{
	SceneNode* selectedNode = nullptr;

	std::queue<SceneNode*> queue;

	size_t intersections = 0;
	static bool addedBoundingBox = false;
	static bool addedLine = false;

	for (const auto& root : scene.getRoots())
	{
		queue.emplace(root);

		while (!queue.empty())
		{
			auto* node = queue.back();
			queue.pop();

			if (node == mBoundingBoxNode) continue;
			if (node == mLineNode) continue;

			auto range = node->getChildren();

			for (auto it = range.begin; it != range.end; ++it)
			{
				queue.emplace(*it);
			}

			auto* mesh = node->getMesh();
			if (mesh != nullptr)
			{
				const auto invModel = inverse(node->getWorldTrafo());
				const auto origin = glm::vec3(invModel * glm::vec4(screenRayWorld.getOrigin(), 1.0f));
				const auto direction = glm::vec3(invModel * glm::vec4(screenRayWorld.getDir(), 0.0f));
				const auto rayLocal = Ray(origin, direction);
				const auto& box = mesh->getAABB();
				const auto result = box.testRayIntersection(rayLocal);
				if (result.intersected && (result.firstIntersection >= 0 || result.secondIntersection >= 0))
				{
					++intersections;
					if (!addedBoundingBox)
					{
						addedBoundingBox = true;
						selectedNode = node;
						auto boxOrigin = (box.max + box.min) / 2.0f;
						glm::mat4 boxTrafo = glm::translate(glm::mat4(1.0f), boxOrigin);
						auto boxScale = (box.max - box.min) / 2.0f;
						boxTrafo = glm::scale(boxTrafo, boxScale);

						mBoundingBoxNode->setLocalTrafo(boxTrafo);
						mBoundingBoxNode->updateWorldTrafoHierarchy();
						mBoundingBoxNode->updateWorldTrafoHierarchy();
						scene.addRoot(mBoundingBoxNode);
					}

					{
						auto lineOrigin = screenRayWorld.getOrigin() + screenRayWorld.getDir() * 0.1f;
						glm::mat4 lineTrafo = glm::translate(glm::mat4(1.0f), lineOrigin);
						auto lineScale = 100.0f * screenRayWorld.getDir();
						lineTrafo = glm::scale(lineTrafo, lineScale);

						mLineNode->setLocalTrafo(lineTrafo);
						mLineNode->updateWorldTrafoHierarchy();
						mLineNode->updateWorldTrafoHierarchy();
					}


					if (!addedLine)
					{
						scene.addRoot(mLineNode);
						addedLine = true;
					}
				}
			}
		}
	}

	std::cout << "Total intersections = " << intersections << std::endl;
	if (intersections == 0)
	{
		scene.removeRoot(mBoundingBoxNode);
		scene.removeRoot(mLineNode);
		addedBoundingBox = false;
		addedLine = false;
	}

	return selectedNode;
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

std::unique_ptr<nex::Mesh> nex::gui::Picker::createBoundingBoxLineMesh()
{
	//create vertices in CCW
	const size_t vertexSize = 8;
	VertexPosition vertices[vertexSize];

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

	const size_t indicesSize = 32;
	unsigned indices[indicesSize];

	// bottom
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 0;
	indices[3] = 3;
	indices[4] = 1;
	indices[5] = 2;
	indices[6] = 2;
	indices[7] = 3;

	// top
	indices[8] = 4;
	indices[9] = 5;
	indices[10] = 4;
	indices[11] = 7;
	indices[12] = 5;
	indices[13] = 6;
	indices[14] = 6;
	indices[15] = 7;

	// front
	indices[16] = 0;
	indices[17] = 4;
	indices[18] = 1;
	indices[19] = 5;

	// back
	indices[20] = 3;
	indices[21] = 7;
	indices[22] = 2;
	indices[23] = 6;

	// left
	indices[24] = 0;
	indices[25] = 4;
	indices[26] = 3;
	indices[27] = 7;

	// right
	indices[28] = 1;
	indices[29] = 5;
	indices[30] = 2;
	indices[31] = 6;

	auto mesh = MeshFactory::createPosition(vertices, vertexSize, indices, indicesSize, { glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX) });
	mesh->mDebugName = "Picker::BoundingBox Line Mesh";
	mesh->setTopology(Topology::LINES);
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