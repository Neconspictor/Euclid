#pragma once
#include <nex/shader/NormalsShader.hpp>
#include <nex/opengl/shader/ShaderGL.hpp>

class NormalsShaderGL : public NormalsShader, public ShaderConfigGL
{
public:
	NormalsShaderGL();

	virtual ~NormalsShaderGL();

	const glm::vec4& getNormalColor() const override;

	void setNormalColor(glm::vec4 color) override;

	void update(const MeshGL& mesh, const TransformData& data) override;

protected:
	glm::vec4 color;
	glm::mat4 transform;
	glm::mat3 normalMatrix;
};