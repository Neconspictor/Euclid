#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>

class PointShadowShaderGL : public ShaderGL
{
public:
	PointShadowShaderGL();

	virtual ~PointShadowShaderGL() = default;

	void setLightPosition(const glm::vec3& pos);
	void setRange(float range);

	// matrices has to be an array of six mat4
	void setShadowMatrices(const glm::mat4* matrices);

	void setModel(const glm::mat4& mat);

	Uniform mModel;
	Uniform mLightPos;
	Uniform mShadowMatrices[6];
	Uniform mRange;
};

class ShadowShaderGL : public ShaderGL
{
public:
	ShadowShaderGL();

	virtual ~ShadowShaderGL() = default;

	void setModel(const glm::mat4& mat);
	void setLightSpaceMatrix(const glm::mat4& mat);

protected:

	Uniform mModel;
	Uniform mLightSpaceMatrix;
};

class VarianceShadowShaderGL : public ShaderGL
{
public:
	VarianceShadowShaderGL();

	virtual ~VarianceShadowShaderGL() = default;

	void setModel(const glm::mat4& mat);
	void setLightSpaceMatrix(const glm::mat4& mat);

protected:
	Uniform mModel;
	Uniform mLightSpaceMatrix;
};