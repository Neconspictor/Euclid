#pragma once

#include <mesh/Mesh.hpp>
#include <glm/glm.hpp>

class Shader
{
public:

	struct TransformData
	{
		glm::mat4 const* projection; 
		glm::mat4 const* view;
		glm::mat4 const* model;
	};

	Shader(){};
	
	virtual ~Shader(){}

	virtual void draw(Mesh const& mesh) = 0;
	
	virtual void drawInstanced(Mesh const& mesh, unsigned int amount) = 0;

	virtual void release() = 0;

	virtual void setTransformData(TransformData data)
	{
		this->data = std::move(data);
	}
	
	virtual void use() = 0;

protected:
	TransformData data;
};