#pragma once

#include <glm/glm.hpp>
#include <material/BlinnPhongMaterial.hpp>
#include <memory>

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
class Mesh
{
public:

	explicit Mesh();
	Mesh(Mesh&& other);
	Mesh& operator=(Mesh&& o);

	virtual ~Mesh();

	unsigned int getIndexSize() const;

	std::reference_wrapper<Material> getMaterial() const;

	void setIndexSize(uint32_t indexSize);
	void setMaterial(std::unique_ptr<Material> material);
protected:
	std::unique_ptr<Material> material;
	uint32_t indexSize;
};