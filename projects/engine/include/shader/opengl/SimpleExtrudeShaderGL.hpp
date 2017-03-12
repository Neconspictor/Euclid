#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/SimpleExtrudeShader.hpp>

class SimpleExtrudeShaderGL : public SimpleExtrudeShader, public ShaderConfigGL
{
public:
	SimpleExtrudeShaderGL();

	virtual ~SimpleExtrudeShaderGL();

	virtual const glm::vec4& getObjectColor() const override;

	virtual void setExtrudeValue(float extrudeValue) override;

	virtual void setObjectColor(glm::vec4 color) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::vec4 objectColor;
	float extrudeValue;
	glm::mat4 transform;
};