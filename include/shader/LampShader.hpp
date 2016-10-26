#ifndef LAMP_SHADER_HPP
#define LAMP_SHADER_HPP

#include <shader/Shader.hpp>
#include <glm/glm.hpp>

class LampShader : public Shader
{
public:
	LampShader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~LampShader();

	virtual void draw(Model const& model, glm::mat4 const& transform) override;
};

#endif