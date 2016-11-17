#ifndef ENGINE_SHADER_OPENGL_SIMPLE_LIGHT_SHADERGL_HPP
#define ENGINE_SHADER_OPENGL_SIMPLE_LIGHT_SHADERGL_HPP
#include <shader/SimpleLightShader.hpp>
#include <shader/opengl/ShaderGL.hpp>

class SimpleLightShaderGL : public ShaderGL, public SimpleLightShader
{
public:
	SimpleLightShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	
	virtual ~SimpleLightShaderGL();
	
	void draw(Model const& model, glm::mat4 const& transform) override;

	glm::vec3 getLightColor() override;

	glm::vec3 getObjectColor() override;

	bool loadingFailed() override;

	void release() override;

	void setLightColor(glm::vec3 color) override;

	void setObjectColor(glm::vec3 color) override;

	void use() override;

private:
	glm::vec3 lightColor;
	glm::vec3 objectColor;
};

#endif