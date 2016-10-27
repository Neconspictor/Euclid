#ifndef BASIC_MODEL_HPP
#define BASIC_MODEL_HPP
#include "model/Model.hpp"

class BasicModel : public Model
{
public:
	explicit BasicModel(GLuint vertexArrayObject, GLuint vertexBufferObject, unsigned int vertexCount)
		: Model(vertexArrayObject, vertexBufferObject, vertexCount){}

	virtual ~BasicModel();
};
#endif
