#pragma once
#include <shader/NormalsShader.hpp>
#include <shader/opengl/ShaderGL.hpp>

class NormalsShaderGL : public NormalsShader
{
public:

	/**
	* Creates a new normals shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	NormalsShaderGL();

	virtual ~NormalsShaderGL();
	
	const ShaderAttribute* getAttributeList() const override;

	const glm::vec4& getNormalColor() const override;

	int getNumberOfAttributes() const override;

	void setNormalColor(glm::vec4 color) override;

	void update(const TransformData& data) override;

protected:
	ShaderAttributeCollection attributes;
	glm::vec4 color;
	glm::mat4 transform;
	glm::mat3 normalMatrix;
};