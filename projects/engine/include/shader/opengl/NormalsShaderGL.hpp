#pragma once
#include <shader/NormalsShader.hpp>
#include <shader/opengl/ShaderGL.hpp>

class NormalsShaderGL : public NormalsShader, ShaderGL
{
public:

	/**
	* Creates a new normals shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	NormalsShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile, 
		const std::string& geometryShaderFile);

	virtual ~NormalsShaderGL();

	void draw(Mesh const& mesh) override;
	
	const glm::vec4& getNormalColor() const override;
	
	void release() override;

	void setNormalColor(glm::vec4 color) override;

	void use() override;

protected:
	glm::vec4 color;
};