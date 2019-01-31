#include <nex/opengl/mesh/MeshFactoryGL.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>

using namespace std;

namespace nex
{

	unique_ptr<MeshGL> MeshFactoryGL::create(const VertexPositionNormalTexTangent* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount)
	{
		using Vertex = VertexPositionNormalTexTangent;


		VertexBuffer vertexBuffer;
		vertexBuffer.bind();
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount);
		indexBuffer.bind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv
		layout.push<glm::vec3>(1); // tangent
		layout.push<glm::vec3>(1); // bitangent

		VertexArray vertexArray;
		vertexArray.addBuffer(std::move(vertexBuffer), layout);

		vertexArray.unbind();
		indexBuffer.unbind();

		return std::make_unique<MeshGL>(std::move(vertexArray), std::move(indexBuffer));
	}

	unique_ptr<MeshGL> MeshFactoryGL::create(const VertexPositionNormalTex * vertices, uint32_t vertexCount, const uint32_t * indices, uint32_t indexCount)
	{
		using Vertex = VertexPositionNormalTex;

		VertexBuffer vertexBuffer;
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount);

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv

		VertexArray vertexArray;
		vertexArray.addBuffer(std::move(vertexBuffer), layout);

		vertexArray.unbind();
		indexBuffer.unbind();

		return std::make_unique<MeshGL>(std::move(vertexArray), std::move(indexBuffer));
	}


	unique_ptr<MeshGL> MeshFactoryGL::createPosition(const VertexPosition* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount)
	{
		using Vertex = VertexPosition;

		VertexBuffer vertexBuffer;
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount);

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position

		VertexArray vertexArray;
		vertexArray.addBuffer(std::move(vertexBuffer), layout);

		vertexArray.unbind();
		indexBuffer.unbind();

		return std::make_unique<MeshGL>(std::move(vertexArray), std::move(indexBuffer));
	}

	unique_ptr<MeshGL> MeshFactoryGL::createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount)
	{
		using Vertex = VertexPositionTex;

		VertexBuffer vertexBuffer;
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount);

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec2>(1); // uv

		VertexArray vertexArray;
		vertexArray.addBuffer(std::move(vertexBuffer), layout);

		vertexArray.unbind();
		indexBuffer.unbind();

		return std::make_unique<MeshGL>(std::move(vertexArray), std::move(indexBuffer));
	}
}