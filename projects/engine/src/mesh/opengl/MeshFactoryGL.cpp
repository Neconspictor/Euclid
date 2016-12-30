#include <mesh/opengl/MeshFactoryGL.hpp>
#include <glad/glad.h>

MeshGL MeshFactoryGL::create(const VertexPositionNormalTex* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount)
{
	using Vertex = VertexPositionNormalTex;

	MeshGL out;
	out.indexSize = indexCount;

	glGenVertexArrays(1, &out.vao);
	glGenBuffers(1, &out.vbo);
	glGenBuffers(1, &out.ebo);

	// 1. bind Vertex Array Object
	glBindVertexArray(out.vao);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
	// 3. copy our indixes in a buffer, too.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	// vertex attribute pointers
	// vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<GLvoid*>(0));

	// vertex normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, normal)));

	// vertex uv coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, texCoords)));

	// clear opengl states
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return out;
}

MeshGL MeshFactoryGL::createPosition(const VertexPosition* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount)
{
	using Vertex = VertexPosition;

	MeshGL out;
	out.indexSize = indexCount;

	glGenVertexArrays(1, &out.vao);
	glGenBuffers(1, &out.vbo);
	glGenBuffers(1, &out.ebo);

	// 1. bind Vertex Array Object
	glBindVertexArray(out.vao);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
	// 3. copy our indixes in a buffer, too.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	// vertex attribute pointers
	// vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<GLvoid*>(0));

	// clear opengl states
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return out;
}

MeshGL MeshFactoryGL::createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount)
{
	using Vertex = VertexPositionTex;

	MeshGL out;
	out.indexSize = indexCount;

	glGenVertexArrays(1, &out.vao);
	glGenBuffers(1, &out.vbo);
	glGenBuffers(1, &out.ebo);

	// 1. bind Vertex Array Object
	glBindVertexArray(out.vao);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
	// 3. copy our indixes in a buffer, too.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	// vertex attribute pointers
	// vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<GLvoid*>(0));

	// vertex uv coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, texCoords)));

	// clear opengl states
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return out;
}