#pragma once

#include <glad/glad.h>
#include <nex/mesh/Mesh.hpp>

class MeshFactoryGL;

class MeshGL : public Mesh
{
public:
	using Vertex = VertexPositionNormalTexTangent;

	MeshGL();
	virtual ~MeshGL();

	GLuint getVertexArrayObject() const;
	GLuint getVertexBufferObject() const;
	GLuint getElementBufferObject() const;

	void setVertexArrayObject(GLuint vao);
	void setVertexBufferObject(GLuint vbo);
	void setElementBufferObject(GLuint ebo);
protected:
	friend MeshFactoryGL; // allow factory for easier access!
	GLuint vao, vbo, ebo;
};
