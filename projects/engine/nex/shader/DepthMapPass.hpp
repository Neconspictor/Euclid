#pragma once
#include <nex/shader/Pass.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/Sampler.hpp>

namespace nex
{
	class CubeDepthMapPass : public Pass
	{
	public:
		CubeDepthMapPass();

		void useCubeDepthMap(const CubeMap* map);

		void setLightPos(const glm::vec3& pos);

		void setRange(float range);

		void setModelMatrix(const glm::mat4& model);
		void setMVP(const glm::mat4& trafo);

	private:
		UniformTex mCubeMap;
		Uniform mLightPos;
		Uniform mRange;
		Uniform mModel;
		Uniform mTransform;
		Sampler mSampler;
	};

	class DepthMapPass : public TransformPass
	{
	public:
		DepthMapPass();

		virtual ~DepthMapPass() = default;

		void useDepthMapTexture(const Texture* texture);

		void setMVP(const glm::mat4& trafo);

		void onTransformUpdate(const TransformData& data) override;
	private:
		UniformTex mDephTexture;
		Uniform mTransform;
		Sampler mSampler;
	};

	class VarianceDepthMapPass : public Pass
	{
	public:
		VarianceDepthMapPass();

		virtual ~VarianceDepthMapPass() = default;

		void useVDepthMapTexture(const Texture* texture);

		void setMVP(const glm::mat4& trafo);

	private:
		UniformTex mDephTexture;
		Uniform mTransform;
		Sampler mSampler;
	};
}
