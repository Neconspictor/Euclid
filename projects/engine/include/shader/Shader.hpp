#ifndef ENGINE_SHADER_SHADER_HPP
#define ENGINE_SHADER_SHADER_HPP

#include <model/Model.hpp>
#include <glm/glm.hpp>

class Shader
{
public:
	Shader(){};
	
	virtual ~Shader(){}

	virtual void draw(Model const& model, glm::mat4 const& projection, glm::mat4 const& view) = 0;

	virtual bool loadingFailed() = 0;
	
	virtual void release() = 0;
	
	virtual void use() = 0;
};

#endif