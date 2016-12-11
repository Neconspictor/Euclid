#ifndef ENGINE_SHADER_OPENGL_SIMPLE_EXTRUDE_SHADERGL_HPP
#define ENGINE_SHADER_OPENGL_SIMPLE_EXTRUDE_SHADERGL_HPP
#include <shader/opengl/ShaderGL.hpp>
#include <shader/SimpleExtrudeShader.hpp>

class SimpleExtrudeShaderGL : public ShaderGL, public SimpleExtrudeShader
{
public:
	/**
	* Creates a new simple extrude shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	SimpleExtrudeShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	virtual ~SimpleExtrudeShaderGL();

	void draw(Mesh const& mesh) override;

	const glm::vec4& getObjectColor() const override;

	void release() override;

	void setExtrudeValue(float extrudeValue) override;

	void setObjectColor(glm::vec4 color) override;

	void use() override;

private:
	glm::vec4 objectColor;
	float extrudeValue;
};

#endif