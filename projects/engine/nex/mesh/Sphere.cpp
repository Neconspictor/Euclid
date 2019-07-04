#include "Sphere.hpp"
#include <cmath>
#include <nex/math/Constant.hpp>
#include "VertexBuffer.hpp"
#include "VertexLayout.hpp"
#include <nex/resource/ResourceLoader.hpp>

namespace nex
{
	SphereMesh::SphereMesh(unsigned int xSegments, unsigned int ySegments)
	{
		ResourceLoader::get()->enqueue([=] {
			init(xSegments, ySegments);
			return this;
		});
	}
	void SphereMesh::init(unsigned int xSegments, unsigned int ySegments)
	{
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 texCoord;
		};

		std::vector<Vertex> vertices;
		std::vector<unsigned> indices;

		for (unsigned int y = 0; y <= ySegments; ++y)
		{
			for (unsigned int x = 0; x <= xSegments; ++x)
			{

				Vertex vertex;

				float xSegment = (float)x / (float)xSegments;
				float ySegment = (float)y / (float)ySegments;
				float xPos = std::cos(xSegment * 2 * PI) * std::sin(ySegment * PI); // TAU is 2PI
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2 * PI) * std::sin(ySegment * PI);

				vertex.position = glm::vec3(xPos, yPos, zPos);
				vertex.texCoord = glm::vec2(xSegment, ySegment);
				vertex.normal = glm::vec3(xPos, yPos, zPos);

				vertices.emplace_back(vertex);
			}
		}

		for (unsigned y = 0; y < ySegments; ++y)
		{
			for (unsigned x = 0; x < xSegments; ++x)
			{
				indices.push_back((y + 1) * (xSegments + 1) + x);
				indices.push_back(y       * (xSegments + 1) + x);
				indices.push_back(y       * (xSegments + 1) + x + 1);

				indices.push_back((y + 1) * (xSegments + 1) + x);
				indices.push_back(y       * (xSegments + 1) + x + 1);
				indices.push_back((y + 1) * (xSegments + 1) + x + 1);
			}
		}


		mVertexBuffer.bind();
		mVertexBuffer.fill(vertices.data(), vertices.size() * sizeof(Vertex));

		mIndexBuffer.bind();
		mIndexBuffer.fill(indices.data(), indices.size(), IndexElementType::BIT_32);
		mIndexBuffer.unbind();

		mLayout.push<glm::vec3>(1); // position
		mLayout.push<glm::vec3>(1); // normal
		mLayout.push<glm::vec2>(1); // uv

		mBoundingBox.min = glm::vec3(-1.0);
		mBoundingBox.max = glm::vec3(1.0);
	}
}