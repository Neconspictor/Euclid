#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <nex/opengl/material/Material.hpp>
#include <memory>

class MeshFactoryGL;

struct VertexPositionNormalTexTangent {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct VertexPositionNormalTex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

struct VertexPosition
{
	glm::vec3 position;
};

struct VertexPositionTex
{
	glm::vec3 position;
	glm::vec2 texCoords;
};


/**
 * Represents a 3d mesh consisting of vertices and a list of indices describing
 * a stream of three sided polygons. A vertex describes the position of a 3d point
 * and can have additional information like a normal and texture uv coordinates.
 * The so-called vertex slice describes of how many (float) data elements a vertex
 * consists of. Theoretically, a vertex isn't bound to floating point units, but this
 * implementation narrows it to floats for ease of use.
 */
class MeshGL
{
public:
	using Vertex = VertexPositionNormalTexTangent;

	MeshGL();
	MeshGL(MeshGL&& other);
	MeshGL& operator=(MeshGL&& o);

	MeshGL(const MeshGL& o) = delete;
	MeshGL& operator=(const MeshGL& o) = delete;

	virtual ~MeshGL();

	GLuint getVertexArrayObject() const;
	GLuint getVertexBufferObject() const;
	GLuint getElementBufferObject() const;

	void setVertexArrayObject(GLuint vao);
	void setVertexBufferObject(GLuint vbo);
	void setElementBufferObject(GLuint ebo);

	unsigned int getIndexSize() const;

	std::reference_wrapper<Material> getMaterial() const;

	void setIndexSize(uint32_t indexSize);
	void setMaterial(std::unique_ptr<Material> material);

protected:
	friend MeshFactoryGL; // allow factory for easier access!
	GLuint vao, vbo, ebo;
	std::unique_ptr<Material> material;
	uint32_t indexSize;
};
