#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>

class SimpleColorShaderGL : public ShaderConfigGL
{
public:
	SimpleColorShaderGL();

	virtual ~SimpleColorShaderGL();

	const glm::vec4& getObjectColor() const;

	void setObjectColor(glm::vec4 color);

	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::vec4 objectColor;
	glm::mat4 transform;
};