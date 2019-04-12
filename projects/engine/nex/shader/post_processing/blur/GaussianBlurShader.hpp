#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{

	class Sampler;

	class GaussianBlurHorizontalShader : public TransformShader
	{
	public:
		GaussianBlurHorizontalShader();

		virtual ~GaussianBlurHorizontalShader() = default;

		void setImageHeight(float height);
		void setImageWidth(float width);

		void setTexture(const Texture* tex);

		void setMVP(const glm::mat4& mvp);


		void onTransformUpdate(const TransformData& data) override;
	protected:
		UniformTex image;
		Uniform transform;
		Uniform windowWidth;
		Uniform windowHeight;
	};

	class GaussianBlurVerticalShader : public TransformShader
	{
	public:
		GaussianBlurVerticalShader();

		virtual ~GaussianBlurVerticalShader() = default;

		void setImageHeight(float height);
		void setImageWidth(float width);

		void setTexture(const Texture* tex);

		void setMVP(const glm::mat4& mvp);

		void onTransformUpdate(const TransformData& data) override;
	protected:
		UniformTex image;
		Uniform transform;
		Uniform windowWidth;
		Uniform windowHeight;
	};
}