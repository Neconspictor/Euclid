#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/shader/SimpleColorShader.hpp>

class SimpleColorShaderGL : public SimpleColorShader, public ShaderConfigGL
{
public:
	SimpleColorShaderGL();

	virtual ~SimpleColorShaderGL();

	virtual const glm::vec4& getObjectColor() const override;

	virtual void setObjectColor(glm::vec4 color) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::vec4 objectColor;
	glm::mat4 transform;
};