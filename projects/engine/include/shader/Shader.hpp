#ifndef SHADER_HPP
#define SHADER_HPP

#include <model/Model.hpp>
#include <glm/glm.hpp>

class Shader
{
public:
	Shader(){};
	
	virtual ~Shader(){}

	virtual void draw(Model const& model, glm::mat4 const& transform) = 0;

	virtual bool loadingFailed() = 0;
	
	virtual void release() = 0;
	
	virtual void use() = 0;
};

#endif