#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>

class PointShadowShaderGL : public ShaderConfigGL
{
public:
	PointShadowShaderGL();

	virtual ~PointShadowShaderGL();

	void setLightPosition(glm::vec3 pos);

	void setRange(float range);

	void setShadowMatrices(glm::mat4 matrices[6]);

	void update(const MeshGL& mesh, const TransformData& data) override;

	glm::vec3 lightPos;
	glm::mat4* matrices;
	float range;
};

class ShadowShaderGL : public ShaderConfigGL
{
public:
	ShadowShaderGL();

	virtual ~ShadowShaderGL();

	void beforeDrawing(const MeshGL& mesh) override;

	void afterDrawing(const MeshGL& mesh) override;


	void update(const MeshGL& mesh, const TransformData& data) override;

protected:
	glm::mat4 lightSpaceMatrix;
};

class VarianceShadowShaderGL : public ShaderConfigGL
{
public:
	VarianceShadowShaderGL();

	virtual ~VarianceShadowShaderGL();

	void update(const MeshGL& mesh, const TransformData& data) override;

protected:
	glm::mat4 lightSpaceMatrix;
};