#ifndef ENGINE_MESH_MESH_HPP
#define ENGINE_MESH_MESH_HPP
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

	explicit Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	Mesh(const Mesh& other);
	Mesh(Mesh&& other);
	Mesh& operator=(const Mesh& o);
	Mesh& operator=(Mesh&& o);

	virtual ~Mesh();

	/**
	 * Provides the index list of this mesh.
	 */
	virtual const std::vector<unsigned int>& getIndices() const;

	virtual Material* getMaterial();
	virtual const Material& getMaterial() const;

	/**
	 * Provides access to the vertex data of this mesh.
	 */
	virtual const std::vector<Vertex>& getVertexData() const;

	virtual void setIndexData(const std::vector<unsigned int>& indices);
	virtual void setIndexData(std::vector<unsigned int>&& indices);

	virtual void setMaterial(Material material);

	virtual void setVertexData(const std::vector<Vertex>& vertices);
	virtual void setVertexData(std::vector<Vertex>&& vertices);

protected:
	std::vector<Vertex> vertexData;
	std::vector<unsigned int> indexData;
	Material material;
};

#endif