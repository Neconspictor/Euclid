#ifndef MODEL_HPP
#define MODEL_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

class Model
{
public:
	Model(GLuint vertexArrayObject, GLuint vertexBufferObject, unsigned int vertexCount);
	virtual ~Model();

	GLuint getVertexArrayObject() const;
	GLuint getVertexBufferObject() const;
	glm::mat4 const& getModelMatrix() const;
	unsigned int getVertexCount() const;


	void setModelMatrix(glm::mat4&& matrix);
	void setModelMatrix(glm::mat4 const& matrix);

	void setVertexArrayObject(GLuint vao);
	void setVertexBufferObject(GLuint vbo);

protected:
	GLuint vao, vbo;
	glm::mat4 modelMatrix;
	unsigned int vertexCount;
};

#endif