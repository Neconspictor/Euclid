#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/SimpleColorShader.hpp>

class SimpleColorShaderGL : public ShaderGL, public SimpleColorShader
{
public:
	/**
	* Creates a new simple color shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	SimpleColorShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	virtual ~SimpleColorShaderGL();

	void draw(Mesh const& mesh) override;

	const glm::vec4& getObjectColor() const override;

	void release() override;

	void setObjectColor(glm::vec4 color) override;

	void use() override;

private:
	glm::vec4 objectColor;
};