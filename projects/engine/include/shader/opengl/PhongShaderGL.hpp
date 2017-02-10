#pragma once
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

	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	const glm::vec3& getLightColor() const override;

	const glm::vec3& getLightPosition() const override;

	void release() override;

	void setLightColor(glm::vec3 color) override;

	void setLightPosition(glm::vec3 position) override;

	void setMaterial(const PhongMaterial& material) override;

	void use() override;

private:
	glm::vec3 lightColor;
	glm::vec3 lightPosition;
	const PhongMaterial* material;
};