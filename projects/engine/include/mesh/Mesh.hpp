#ifndef ENGINE_MESH_MESH_HPP
#define ENGINE_MESH_MESH_HPP
#include <vector>

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
	Mesh();
	explicit Mesh(std::vector<float> vertices, size_t vertexSliceSize, std::vector<size_t> indices);
	Mesh(const Mesh& other);
	Mesh(Mesh&& other);

	virtual ~Mesh();

	/**
	 * Provides the index list of this mesh.
	 */
	const std::vector<size_t>& getIndices() const;

	/**
	 * Provides access to the vertex data of this mesh.
	 */
	const float* getVertexData() const;

	/**
	 * Provides the vertex slice of this mesh.
	 */
	size_t getVertexSlice() const;

	/**
	 * Sets the mesh data of this mesh.
	 */
	void setContent(std::vector<float> vertices, size_t vertexSliceSize, std::vector<size_t> indices);

protected:
	std::vector<float> vertexData;
	size_t vertexSlice;
	std::vector<size_t> indexData;
};

#endif