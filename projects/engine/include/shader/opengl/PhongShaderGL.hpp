#ifndef ENGINE_SHADER_OPENGL_PHONG_SHADERGL_HPP
#define ENGINE_SHADER_OPENGL_PHONG_SHADERGL_HPP
#include <shader/opengl/ShaderGL.hpp>
#include <shader/PhongShader.hpp>

class PhongShaderGL : public ShaderGL, public PhongShader
{
public:
	/**
	* Creates a new phong shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	PhongShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	virtual ~PhongShaderGL();

	void draw(Model const& model, glm::mat4 const& projection, glm::mat4 const& view) override;

	const glm::vec3& getLightColor() const override;

	const glm::vec3& getLightPosition() const override;

	const glm::vec3& getObjectColor() const override;

	bool loadingFailed() override;

	void release() override;

	void setLightColor(glm::vec3 color) override;

	void setLightPosition(glm::vec3 position) override;

	void setObjectColor(glm::vec3 color) override;

	void use() override;

private:
	glm::vec3 lightColor;
	glm::vec3 lightPosition;
	glm::vec3 objectColor;
};
#endif