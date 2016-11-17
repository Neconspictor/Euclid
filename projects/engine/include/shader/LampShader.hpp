#ifndef LAMP_SHADER_HPP
#define LAMP_SHADER_HPP

#include <shader/Shader.hpp>

class LampShader : public Shader
{
public:
	LampShader() : Shader() {}
	virtual ~LampShader() {};
};

#endif