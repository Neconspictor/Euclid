#ifndef ENGINE_SHADER_SHADER_HPP
#define ENGINE_SHADER_SHADER_HPP

#include <mesh/Mesh.hpp>
#include <glm/glm.hpp>

class Shader
{
public:

	struct TransformData
	{
		glm::mat4* projection; 
		glm::mat4* view;
		glm::mat4* model;
	};

	Shader(){};
	
	virtual ~Shader(){}

	virtual void draw(Mesh const& mesh) = 0;

	virtual bool loadingFailed() = 0;
	
	virtual void release() = 0;

	virtual void setTransformData(TransformData data)
	{
		this->data = std::move(data);
	}
	
	virtual void use() = 0;

protected:
	TransformData data;
};

#endif