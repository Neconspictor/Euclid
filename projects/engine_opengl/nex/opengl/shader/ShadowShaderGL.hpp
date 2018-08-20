#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/shader/ShadowShader.hpp>

class PointShadowShaderGL : public PointShadowShader, public ShaderConfigGL
{
public:
	PointShadowShaderGL();

	virtual ~PointShadowShaderGL();

	virtual void setLightPosition(glm::vec3 pos) override;

	virtual void setRange(float range) override;

	virtual void setShadowMatrices(glm::mat4 matrices[6]) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

	glm::vec3 lightPos;
	glm::mat4* matrices;
	float range;
};

class ShadowShaderGL : public ShadowShader, public ShaderConfigGL
{
public:
	ShadowShaderGL();

	virtual ~ShadowShaderGL();

	virtual void beforeDrawing(const MeshGL& mesh) override;

	virtual void afterDrawing(const MeshGL& mesh) override;


	virtual void update(const MeshGL& mesh, const TransformData& data) override;

protected:
	glm::mat4 lightSpaceMatrix;
};

class VarianceShadowShaderGL : public VarianceShadowShader, public ShaderConfigGL
{
public:
	VarianceShadowShaderGL();

	virtual ~VarianceShadowShaderGL();

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

protected:
	glm::mat4 lightSpaceMatrix;
};