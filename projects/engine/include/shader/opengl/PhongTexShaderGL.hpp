#ifndef ENGINE_SHADER_OPENGL_PHONG_TEX_SHADERGL_HPP
#define ENGINE_SHADER_OPENGL_PHONG_TEX_SHADERGL_HPP
#include <shader/opengl/ShaderGL.hpp>
#include <shader/PhongTextureShader.hpp>

class PhongTexShaderGL : public ShaderGL, public PhongTextureShader
{
public:
	/**
	* Creates a new phong shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	PhongTexShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	virtual ~PhongTexShaderGL();

	void draw(Model const& model, glm::mat4 const& projection, glm::mat4 const& view) override;

	const glm::vec3& getLightColor() const override;

	const glm::vec3& getLightPosition() const override;

	bool loadingFailed() override;

	void release() override;

	void setLightColor(glm::vec3 color) override;

	void setLightPosition(glm::vec3 position) override;

	void setMaterial(const PhongTexMaterial& material) override;

	void setPointLightPositions(glm::vec3* positions) override;

	void setSpotLightDiection(glm::vec3 direction) override;

	void use() override;

	void setViewPosition(glm::vec3 position) override;

private:

	void initLights();

	glm::vec3 lightColor;
	glm::vec3 lightPosition;
	glm::vec3 viewPosition;
	glm::vec3 spotLightDirection;
	glm::vec3 pointLightPositions[4];
	const PhongTexMaterial* material;
};
#endif