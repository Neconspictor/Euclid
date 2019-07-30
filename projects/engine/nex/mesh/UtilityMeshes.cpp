#include <nex/mesh/UtilityMeshes.hpp>
#include <cmath>
#include <nex/math/Constant.hpp>
#include "VertexBuffer.hpp"
#include "VertexLayout.hpp"
#include <nex/resource/ResourceLoader.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/math/Math.hpp>

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

		// define layout
		mLayout.push<glm::vec3>(1); // position
		mLayout.push<glm::vec3>(1); // normal
		mLayout.push<glm::vec2>(1); // uv

		// calc bounding box
		mBoundingBox.min = glm::vec3(-1.0);
		mBoundingBox.max = glm::vec3(1.0);


		// upload data into buffers
		mVertexBuffer.bind();
		mVertexBuffer.fill(vertices.data(), vertices.size() * sizeof(VertexPositionNormalTex));

		mIndexBuffer.bind();
		mIndexBuffer.fill(indices.data(), indices.size(), IndexElementType::BIT_32);
		mIndexBuffer.unbind();



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

		// front side triangles
		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);

		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);

		// back side triangles
		indices.push_back((unsigned)FrustumCorners::FarRightBottom);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);

		indices.push_back((unsigned)FrustumCorners::FarRightBottom);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);


		// left side triangles
		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);

		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);
		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);

		// right side triangles
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);

		indices.push_back((unsigned)FrustumCorners::NearRightBottom);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);
		indices.push_back((unsigned)FrustumCorners::FarRightBottom);

		// top side triangles
		indices.push_back((unsigned)FrustumCorners::NearLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);

		indices.push_back((unsigned)FrustumCorners::NearLeftTop);
		indices.push_back((unsigned)FrustumCorners::FarRightTop);
		indices.push_back((unsigned)FrustumCorners::NearRightTop);

		// bottom side triangles
		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);

		indices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		indices.push_back((unsigned)FrustumCorners::NearRightBottom);
		indices.push_back((unsigned)FrustumCorners::FarRightBottom);


		// Calc bounding box
		for (auto i = 0; i < 8; ++i) {
			mBoundingBox.min = nex::minVec(mBoundingBox.min, vertices[i].position);
			mBoundingBox.max = nex::maxVec(mBoundingBox.max, vertices[i].position);
		}

		// define layout
		mLayout.push<glm::vec3>(1); // position
		mLayout.push<glm::vec3>(1); // normal
		mLayout.push<glm::vec2>(1); // uv


		// upload data into buffers
		mVertexBuffer.bind();
		mVertexBuffer.fill(vertices, 8 * sizeof(VertexPositionNormalTex));

		mIndexBuffer.bind();
		mIndexBuffer.fill(indices.data(), indices.size(), IndexElementType::BIT_32);
		mIndexBuffer.unbind();
	}
}