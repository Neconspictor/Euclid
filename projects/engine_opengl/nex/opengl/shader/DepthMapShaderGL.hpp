#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
	class CubeDepthMapShader : public Shader
	{
	public:
		CubeDepthMapShader();

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
	};

	class DepthMapShader : public TransformShader
	{
	public:
		DepthMapShader();

		virtual ~DepthMapShader() = default;

		void useDepthMapTexture(const Texture* texture);

		void setMVP(const glm::mat4& trafo);

		void onTransformUpdate(const TransformData& data) override;
	private:
		UniformTex mDephTexture;
		Uniform mTransform;
	};

	class VarianceDepthMapShader : public Shader
	{
	public:
		VarianceDepthMapShader();

		virtual ~VarianceDepthMapShader() = default;

		void useVDepthMapTexture(const Texture* texture);

		void setMVP(const glm::mat4& trafo);

	private:
		UniformTex mDephTexture;
		Uniform mTransform;
	};
}