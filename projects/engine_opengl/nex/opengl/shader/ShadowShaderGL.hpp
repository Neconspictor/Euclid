#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>

namespace nex
{
	class PointShadowShader : public Shader
	{
	public:
		PointShadowShader();

		virtual ~PointShadowShader() = default;

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

	class ShadowShader : public Shader
	{
	public:
		ShadowShader();

		virtual ~ShadowShader() = default;

		void setModel(const glm::mat4& mat);
		void setLightSpaceMatrix(const glm::mat4& mat);

		void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;

	protected:

		Uniform mModel;
		Uniform mLightSpaceMatrix;
	};

	class VarianceShadowShader : public Shader
	{
	public:
		VarianceShadowShader();

		virtual ~VarianceShadowShader() = default;

		void setModel(const glm::mat4& mat);
		void setLightSpaceMatrix(const glm::mat4& mat);

	protected:
		Uniform mModel;
		Uniform mLightSpaceMatrix;
	};
}