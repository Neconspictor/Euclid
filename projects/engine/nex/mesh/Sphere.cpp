#include "Sphere.hpp"
#include <math.h>
#include <nex/util/Math.hpp>

namespace nex
{
	Sphere::Sphere(unsigned int xSegments, unsigned int ySegments, Material* material)
	{
		setMaterial(material);


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
				float xPos = std::cos(xSegment * 2 * util::PI) * std::sin(ySegment * util::PI); // TAU is 2PI
				float yPos = std::cos(ySegment * util::PI);
				float zPos = std::sin(xSegment * 2 * util::PI) * std::sin(ySegment * util::PI);

				vertex.position = glm::vec3(xPos, yPos, zPos);
				vertex.texCoord = glm::vec2(xSegment, ySegment);
				vertex.normal = glm::vec3(xPos, yPos, zPos);

				vertices.emplace_back(vertex);
			}
		}

		bool oddRow = false;
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




		VertexBuffer vertexBuffer;
		vertexBuffer.bind();
		vertexBuffer.fill(vertices.data(), vertices.size() * sizeof(Vertex));

		mIndexBuffer.bind();
		mIndexBuffer.fill(indices.data(), static_cast<unsigned>(indices.size()));

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv


		mVertexArray.bind();
		mVertexArray.addBuffer(std::move(vertexBuffer), layout);

		mVertexArray.unbind();
		mIndexBuffer.unbind();

	}
}