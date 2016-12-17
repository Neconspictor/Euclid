#pragma once
#include <glm/glm.hpp>
#include <material/Material.hpp>


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
	Mesh(const Mesh& other);
	Mesh(Mesh&& other);
	Mesh& operator=(const Mesh& o);
	Mesh& operator=(Mesh&& o);

	virtual ~Mesh();

	unsigned int getIndexSize() const;
	
	Material* getMaterial();
	const Material& getMaterial() const;

	void setIndexSize(uint32_t indexSize);
	void setMaterial(Material material);
protected:
	Material material;
	uint32_t indexSize;
};