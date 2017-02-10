#pragma once
#include <shader/SimpleLightShader.hpp>
#include <shader/opengl/ShaderGL.hpp>

class SimpleLightShaderGL : public ShaderGL, public SimpleLightShader
{
public:
	/**
	* Creates a new simple light shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	SimpleLightShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	
	virtual ~SimpleLightShaderGL();
	
	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	const glm::vec3& getLightColor() const override;

	const glm::vec4& getObjectColor() const override;

	void release() override;

	void setLightColor(glm::vec3 color) override;

	void setObjectColor(glm::vec4 color) override;

	void use() override;

private:
	glm::vec3 lightColor;
	glm::vec4 objectColor;
};