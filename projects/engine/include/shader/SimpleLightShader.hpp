#ifndef SIMPLE_LIGHT_SHADER_HPP
#define SIMPLE_LIGHT_SHADER_HPP
#include "shader/shader.hpp"
#include <glm/glm.hpp>

class SimpleLightShader : public Shader
{
public:
	SimpleLightShader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~SimpleLightShader();

	glm::vec3 getObjectColor();
	glm::vec3 getLightColor();

	void setLightColor(glm::vec3 color);
	void setObjectColor(glm::vec3 color);

	virtual void release() override;
	virtual void use() override;
	virtual void draw(Model const& model, glm::mat4 const& trans) override;
private:
	glm::vec3 lightColor;
	glm::vec3 objectColor;
};
#endif