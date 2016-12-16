#pragma once

#include <GL/glew.h>
#include <mesh/Mesh.hpp>

class MeshGL : public Mesh
{
public:
	MeshGL(const Vertex* vertices, unsigned int vertexCount, const unsigned int* indices, unsigned int indexCount);
	virtual ~MeshGL();

	GLuint getVertexArrayObject() const;
	GLuint getVertexBufferObject() const;

	void setVertexArrayObject(GLuint vao);
	void setVertexBufferObject(GLuint vbo);
protected:
	GLuint vao, vbo, ebo;
};