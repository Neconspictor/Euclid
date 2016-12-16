#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <material/Material.hpp>

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

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
	};

	explicit Mesh(unsigned int indexSize);
	Mesh(const Mesh& other);
	Mesh(Mesh&& other);
	Mesh& operator=(const Mesh& o);
	Mesh& operator=(Mesh&& o);

	virtual ~Mesh();

	unsigned int getIndexSize() const;
	
	Material* getMaterial();
	const Material& getMaterial() const;

	void setMaterial(Material material);
protected:
	Material material;
	unsigned int indexSize;
};