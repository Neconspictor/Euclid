#ifndef ENGINE_MODEL_OPENGL_MESHGL_HPP
#define ENGINE_MODEL_OPENGL_MESHGL_HPP

#include <GL/glew.h>
#include <mesh/Mesh.hpp>

class MeshGL : public Mesh
{
public:
	MeshGL(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	virtual ~MeshGL();

	GLuint getVertexArrayObject() const;
	GLuint getVertexBufferObject() const;

	void setVertexArrayObject(GLuint vao);
	void setVertexBufferObject(GLuint vbo);
protected:
	GLuint vao, vbo, ebo;
};

#endif