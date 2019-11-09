#include <nex/mesh/UtilityMeshes.hpp>
#include <cmath>
#include <nex/math/Constant.hpp>
#include <nex/buffer/VertexBuffer.hpp>
#include "VertexLayout.hpp"
#include <nex/resource/ResourceLoader.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/math/Math.hpp>
#include <nex/math/BoundingBox.hpp>

namespace nex
{
	SphereMesh::SphereMesh(unsigned int xSegments, unsigned int ySegments, bool finalize)
	{
		std::vector<VertexPositionNormalTex> vertices;
		std::vector<unsigned> indices;

		for (unsigned int y = 0; y <= ySegments; ++y)
		{
			for (unsigned int x = 0; x <= xSegments; ++x)
			{

				VertexPositionNormalTex vertex;

				float xSegment = (float)x / (float)xSegments;
				float ySegment = (float)y / (float)ySegments;
				float xPos = std::cos(xSegment * 2 * PI) * std::sin(ySegment * PI); // TAU is 2PI
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2 * PI) * std::sin(ySegment * PI);

				vertex.position = glm::vec3(xPos, yPos, zPos);
				vertex.texCoords = glm::vec2(xSegment, ySegment);
				vertex.normal = glm::vec3(xPos, yPos, zPos);

				vertices.emplace_back(vertex);
			}
		}

		for (unsigned y = 0; y < ySegments; ++y)
		{
			for (unsigned x = 0; x < xSegments; ++x)
			{
				indices.push_back((y + 1) * (xSegments + 1) + x);
				indices.push_back(y * (xSegments + 1) + x);
				indices.push_back(y * (xSegments + 1) + x + 1);

				indices.push_back((y + 1) * (xSegments + 1) + x);
				indices.push_back(y * (xSegments + 1) + x + 1);
				indices.push_back((y + 1) * (xSegments + 1) + x + 1);
			}
		}

		

		// calc bounding box
		mBoundingBox.min = glm::vec3(-1.0);
		mBoundingBox.max = glm::vec3(1.0);


		// upload data into buffers
		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(vertices.size() * sizeof(VertexPositionNormalTex), vertices.data(), GpuBuffer::UsageHint::STATIC_DRAW);
		mIndexBuffer.fill(IndexElementType::BIT_32, indices.size(), indices.data());

		// define layout
		mLayout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // position
		mLayout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // normal
		mLayout.push<glm::vec2>(1, vertexBuffer.get(), false, false); // uv

		addVertexDataBuffer(std::move(vertexBuffer));



		if (!finalize) return;
		ResourceLoader::get()->enqueue([=](RenderEngine::CommandQueue* commandQueue)->nex::Resource* {
			commandQueue->push([=]() {
				this->finalize();
			});
			return this;
		});
	}


	FrustumMesh::FrustumMesh(const Frustum& frustum)
	{
		std::vector<unsigned> indices;

		/**
		 * corner order:
		 * 0: FarLeftBottom
		 * 1: FarLeftTop
		 * 2: FarRightBottom
		 * 3: FarRightTop
		 * 4: NearLeftBottom
		 * 5: NearLeftTop
		 * 6: NearRightBottom
		 * 7: NearRightTop
		 */
		VertexPositionNormalTex vertices[8];

		glm::vec3 normalRight(1, 0, 0);
		glm::vec3 normalLeft = -normalRight;
		glm::vec3 normalTop(0,1,0);
		glm::vec3 normalBottom = -normalTop;
		glm::vec3 normalFront(0, 0, -1);
		glm::vec3 normalBack = -normalFront;

		auto id = (unsigned)FrustumCorners::FarLeftBottom;
		vertices[id].position = frustum.corners[id];
		vertices[id].normal = normalize((normalFront + normalBottom + normalLeft) / 3.0f);
		vertices[id].texCoords = glm::vec2(0.0f); //TODO

		id = (unsigned)FrustumCorners::FarLeftTop;
		vertices[id].position = frustum.corners[id];
		vertices[id].normal = normalize((normalFront + normalTop + normalLeft) / 3.0f);
		vertices[id].texCoords = glm::vec2(0.0f); //TODO

		id = (unsigned)FrustumCorners::FarRightBottom;
		vertices[id].position = frustum.corners[id];
		vertices[id].normal = normalize((normalFront + normalBottom + normalRight) / 3.0f);
		vertices[id].texCoords = glm::vec2(0.0f); //TODO

		id = (unsigned)FrustumCorners::FarRightTop;
		vertices[id].position = frustum.corners[id];
		vertices[id].normal = normalize((normalFront + normalTop + normalRight) / 3.0f);
		vertices[id].texCoords = glm::vec2(0.0f); //TODO

		id = (unsigned)FrustumCorners::NearLeftBottom;
		vertices[id].position = frustum.corners[id];
		vertices[id].normal = normalize((normalBack + normalBottom + normalLeft) / 3.0f);
		vertices[id].texCoords = glm::vec2(0.0f); //TODO

		id = (unsigned)FrustumCorners::NearLeftTop;
		vertices[id].position = frustum.corners[id];
		vertices[id].normal = normalize((normalBack + normalTop + normalLeft) / 3.0f);
		vertices[id].texCoords = glm::vec2(0.0f); //TODO

		id = (unsigned)FrustumCorners::NearRightBottom;
		vertices[id].position = frustum.corners[id];
		vertices[id].normal = normalize((normalBack + normalBottom + normalRight) / 3.0f);
		vertices[id].texCoords = glm::vec2(0.0f); //TODO

		id = (unsigned)FrustumCorners::NearRightTop;
		vertices[id].position = frustum.corners[id];
		vertices[id].normal = normalize((normalBack + normalTop + normalRight) / 3.0f);
		vertices[id].texCoords = glm::vec2(0.0f); //TODO

		// defines indices

		// front side
		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);
		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);

		// back side
		indices.push_back((unsigned)FrustumCorners::FarRightBottom);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		indices.push_back((unsigned)FrustumCorners::FarRightBottom);

		// left side
		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);
		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);

		// right side
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);
		indices.push_back((unsigned)FrustumCorners::FarRightBottom);
		indices.push_back((unsigned)FrustumCorners::FarRightBottom);
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);

		// top side
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);

		// bottom side
		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);
		indices.push_back((unsigned)FrustumCorners::FarRightBottom);
		indices.push_back((unsigned)FrustumCorners::FarRightBottom);
		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);


		// Calc bounding box
		for (auto i = 0; i < 8; ++i) {
			mBoundingBox.min = nex::minVec(mBoundingBox.min, vertices[i].position);
			mBoundingBox.max = nex::maxVec(mBoundingBox.max, vertices[i].position);
		}


		// upload data into buffers
		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(8 * sizeof(VertexPositionNormalTex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		mIndexBuffer.fill(IndexElementType::BIT_32, indices.size(), indices.data());

		// define layout
		mLayout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // position
		mLayout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // normal
		mLayout.push<glm::vec2>(1, vertexBuffer.get(), false, false); // uv

		addVertexDataBuffer(std::move(vertexBuffer));


		mTopology = Topology::LINES;
	}
	MeshAABB::MeshAABB(const AABB& box, nex::Topology topology)
	{

		if (topology == Topology::LINES) {
			createLineMesh(box);
		}
		else {
			createTriangleMesh(box);
		}
	}

	void MeshAABB::createLineMesh(const AABB& box)
	{
		//create vertices in CCW
		const size_t vertexSize = 8;
		VertexPosition vertices[vertexSize];

		// bottom plane
		vertices[0].position = box.min;
		vertices[1].position = glm::vec3(box.min.x, box.min.y, box.max.z);
		vertices[2].position = glm::vec3(box.max.x, box.min.y, box.max.z);
		vertices[3].position = glm::vec3(box.max.x, box.min.y, box.min.z);

		// top plane
		vertices[4].position = glm::vec3(box.min.x, box.max.y, box.min.z);
		vertices[5].position = glm::vec3(box.min.x, box.max.y, box.max.z);
		vertices[6].position = box.max;
		vertices[7].position = glm::vec3(box.max.x, box.max.y, box.min.z);

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


		// Calc bounding box
		mBoundingBox = box;


		// upload data into buffers
		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(vertexSize * sizeof(VertexPosition), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		mIndexBuffer.fill(IndexElementType::BIT_32, indicesSize, indices);

		// define layout
		mLayout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // position

		addVertexDataBuffer(std::move(vertexBuffer));

		mTopology = Topology::LINES;
	}
	void MeshAABB::createTriangleMesh(const AABB& box)
	{
		//create vertices in CCW
		const size_t vertexSize = 8;
		VertexPosition vertices[vertexSize];

		// bottom plane
		vertices[0].position = box.min; // bottom left front
		vertices[1].position = glm::vec3(box.min.x, box.min.y, box.max.z); // bottom left back
		vertices[2].position = glm::vec3(box.max.x, box.min.y, box.max.z); // bottom right back
		vertices[3].position = glm::vec3(box.max.x, box.min.y, box.min.z); // bottom right front

		// top plane
		vertices[4].position = glm::vec3(box.min.x, box.max.y, box.min.z); // top left front
		vertices[5].position = glm::vec3(box.min.x, box.max.y, box.max.z); // top left back
		vertices[6].position = box.max; // top right back
		vertices[7].position = glm::vec3(box.max.x, box.max.y, box.min.z); // top right front

		const size_t indicesSize = 36;
		unsigned indices[indicesSize];

		// bottom
		indices[0] = 1; // bottom left back
		indices[1] = 0; // bottom left front
		indices[2] = 3; // bottom right front
		indices[3] = 1; // bottom left back
		indices[4] = 3; // bottom right front
		indices[5] = 2; // bottom right back

		// top
		indices[6] = 4;  // top left front
		indices[7] = 5;  // top left back
		indices[8] = 6;  // top right back
		indices[9] = 4;  // top left front
		indices[10] = 6; // top right back
		indices[11] = 7; // top right front

		// front
		indices[12] = 0;  // bottom left front
		indices[13] = 4;  // top left front
		indices[14] = 7;  // top right front
		indices[15] = 0;  // bottom left front
		indices[16] = 7; // top right front
		indices[17] = 3; // bottom right front

		// back
		indices[18] = 2;  // bottom right back
		indices[19] = 6;  // top right back
		indices[20] = 5;  // top left back
		indices[21] = 2;  // bottom right back
		indices[22] = 5; // top left back
		indices[23] = 1; // bottom left back

		// left
		indices[24] = 1;  // bottom left back
		indices[25] = 5;  // top left back
		indices[26] = 4;  // top left front
		indices[27] = 1;  // bottom left back
		indices[28] = 4; // top left front
		indices[29] = 0; // bottom left front

		// right
		indices[30] = 3;  // bottom right front
		indices[31] = 7;  // top right front
		indices[32] = 6;  // top right back
		indices[33] = 3;  // bottom right front
		indices[34] = 6; // top right back
		indices[35] = 2; // bottom right back


		// Calc bounding box
		mBoundingBox = box;

		// upload data into buffers
		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(vertexSize * sizeof(VertexPosition), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		mIndexBuffer.fill(IndexElementType::BIT_32, indicesSize, indices);

		// define layout
		mLayout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // position

		addVertexDataBuffer(std::move(vertexBuffer));

		mTopology = Topology::TRIANGLES;
	}
}