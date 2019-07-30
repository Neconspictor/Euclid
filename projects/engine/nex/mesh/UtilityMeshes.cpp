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
	SphereMesh::SphereMesh(unsigned int xSegments, unsigned int ySegments, bool finalize) : mXSegments(xSegments),
		mYSegments(ySegments)
	{

		for (unsigned int y = 0; y <= mYSegments; ++y)
		{
			for (unsigned int x = 0; x <= mXSegments; ++x)
			{

				VertexPositionNormalTex vertex;

				float xSegment = (float)x / (float)mXSegments;
				float ySegment = (float)y / (float)mYSegments;
				float xPos = std::cos(xSegment * 2 * PI) * std::sin(ySegment * PI); // TAU is 2PI
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2 * PI) * std::sin(ySegment * PI);

				vertex.position = glm::vec3(xPos, yPos, zPos);
				vertex.texCoords = glm::vec2(xSegment, ySegment);
				vertex.normal = glm::vec3(xPos, yPos, zPos);

				mVertices.emplace_back(vertex);
			}
		}

		for (unsigned y = 0; y < mYSegments; ++y)
		{
			for (unsigned x = 0; x < mXSegments; ++x)
			{
				mIndices.push_back((y + 1) * (mXSegments + 1) + x);
				mIndices.push_back(y * (mXSegments + 1) + x);
				mIndices.push_back(y * (mXSegments + 1) + x + 1);

				mIndices.push_back((y + 1) * (mXSegments + 1) + x);
				mIndices.push_back(y * (mXSegments + 1) + x + 1);
				mIndices.push_back((y + 1) * (mXSegments + 1) + x + 1);
			}
		}

		// define layout
		mLayout.push<glm::vec3>(1); // position
		mLayout.push<glm::vec3>(1); // normal
		mLayout.push<glm::vec2>(1); // uv

		// calc bounding box
		mBoundingBox.min = glm::vec3(-1.0);
		mBoundingBox.max = glm::vec3(1.0);



		if (!finalize) return;
		ResourceLoader::get()->enqueue([=](RenderEngine::CommandQueue* commandQueue)->nex::Resource* {
			commandQueue->push([=]() {
				this->finalize();
			});
			return this;
		});
	}
	void SphereMesh::finalize()
	{
		mVertexBuffer.bind();
		mVertexBuffer.fill(mVertices.data(), mVertices.size() * sizeof(VertexPositionNormalTex));

		mIndexBuffer.bind();
		mIndexBuffer.fill(mIndices.data(), mIndices.size(), IndexElementType::BIT_32);
		mIndexBuffer.unbind();

		Mesh::finalize();

		mVertices.clear();
		mVertices.shrink_to_fit();
		mIndices.clear();
		mIndices.shrink_to_fit();
	}


	FrustumMesh::FrustumMesh(const Frustum& frustum)
	{

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
		mIndices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		mIndices.push_back((unsigned)FrustumCorners::NearLeftTop);
		mIndices.push_back((unsigned)FrustumCorners::NearRightTop);

		mIndices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		mIndices.push_back((unsigned)FrustumCorners::NearRightTop);
		mIndices.push_back((unsigned)FrustumCorners::NearRightBottom);

		// back side triangles
		mIndices.push_back((unsigned)FrustumCorners::FarRightBottom);
		mIndices.push_back((unsigned)FrustumCorners::FarRightTop);
		mIndices.push_back((unsigned)FrustumCorners::FarLeftTop);

		mIndices.push_back((unsigned)FrustumCorners::FarRightBottom);
		mIndices.push_back((unsigned)FrustumCorners::FarLeftTop);
		mIndices.push_back((unsigned)FrustumCorners::FarLeftBottom);


		// left side triangles
		mIndices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		mIndices.push_back((unsigned)FrustumCorners::FarLeftTop);
		mIndices.push_back((unsigned)FrustumCorners::NearLeftTop);

		mIndices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		mIndices.push_back((unsigned)FrustumCorners::NearLeftTop);
		mIndices.push_back((unsigned)FrustumCorners::NearLeftBottom);

		// right side triangles
		mIndices.push_back((unsigned)FrustumCorners::NearRightBottom);
		mIndices.push_back((unsigned)FrustumCorners::NearRightTop);
		mIndices.push_back((unsigned)FrustumCorners::FarRightTop);

		mIndices.push_back((unsigned)FrustumCorners::NearRightBottom);
		mIndices.push_back((unsigned)FrustumCorners::FarRightTop);
		mIndices.push_back((unsigned)FrustumCorners::FarRightBottom);

		// top side triangles
		mIndices.push_back((unsigned)FrustumCorners::NearLeftTop);
		mIndices.push_back((unsigned)FrustumCorners::FarLeftTop);
		mIndices.push_back((unsigned)FrustumCorners::FarRightTop);

		mIndices.push_back((unsigned)FrustumCorners::NearLeftTop);
		mIndices.push_back((unsigned)FrustumCorners::FarRightTop);
		mIndices.push_back((unsigned)FrustumCorners::NearRightTop);

		// bottom side triangles
		mIndices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		mIndices.push_back((unsigned)FrustumCorners::NearLeftBottom);
		mIndices.push_back((unsigned)FrustumCorners::NearRightBottom);

		mIndices.push_back((unsigned)FrustumCorners::FarLeftBottom);
		mIndices.push_back((unsigned)FrustumCorners::NearRightBottom);
		mIndices.push_back((unsigned)FrustumCorners::FarRightBottom);


		// Calc bounding box
		for (auto i = 0; i < 8; ++i) {
			mBoundingBox.min = nex::minVec(mBoundingBox.min, mVertices[i].position);
			mBoundingBox.max = nex::maxVec(mBoundingBox.max, mVertices[i].position);
		}

		// define layout
		mLayout.push<glm::vec3>(1); // position
		mLayout.push<glm::vec3>(1); // normal
		mLayout.push<glm::vec2>(1); // uv

	}
	void FrustumMesh::finalize()
	{
		mVertexBuffer.bind();
		mVertexBuffer.fill(mVertices.data(), mVertices.size() * sizeof(VertexPositionNormalTex));

		mIndexBuffer.bind();
		mIndexBuffer.fill(mIndices.data(), mIndices.size(), IndexElementType::BIT_32);
		mIndexBuffer.unbind();


		Mesh::finalize();

		mVertices.clear();
		mVertices.shrink_to_fit();
		mIndices.clear();
		mIndices.shrink_to_fit();
	}
}