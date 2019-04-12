#pragma once
#include <nex/shader/Pass.hpp>

namespace nex
{
	class PointShadowPass : public Pass
	{
	public:
		PointShadowPass();

		virtual ~PointShadowPass() = default;

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

	class ShadowPass : public Pass
	{
	public:
		ShadowPass();

		virtual ~ShadowPass() = default;

		void setModel(const glm::mat4& mat);
		void setLightSpaceMatrix(const glm::mat4& mat);

		void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;

	protected:

		Uniform mModel;
		Uniform mLightSpaceMatrix;
	};

	class VarianceShadowPass : public Pass
	{
	public:
		VarianceShadowPass();

		virtual ~VarianceShadowPass() = default;

		void setModel(const glm::mat4& mat);
		void setLightSpaceMatrix(const glm::mat4& mat);

	protected:
		Uniform mModel;
		Uniform mLightSpaceMatrix;
	};
}