#ifndef ENGINE_MODEL_OPENGL_MESHGL_HPP
#define ENGINE_MODEL_OPENGL_MESHGL_HPP

#include <GL/glew.h>
#include <mesh/Mesh.hpp>

class MeshGL : public Mesh
{
public:
	MeshGL(std::vector<float> vertices, size_t vertexSliceSize, std::vector<size_t> indices, 
		GLuint vertexArrayObject, GLuint vertexBufferObject, unsigned int vertexCount);
	virtual ~MeshGL();

	unsigned int getVertexCount() const;

	GLuint getVertexArrayObject() const;
	GLuint getVertexBufferObject() const;

	void setVertexArrayObject(GLuint vao);
	void setVertexBufferObject(GLuint vbo);

protected:
	GLuint vao, vbo;
	unsigned int vertexCount;
};

#endif