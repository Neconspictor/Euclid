#pragma once
#include <nex/shader/Pass.hpp>

namespace nex
{

	class Sampler;

	class GaussianBlurHorizontalShader : public Pass
	{
	public:
		GaussianBlurHorizontalShader();

		virtual ~GaussianBlurHorizontalShader() = default;

		void setImageHeight(float height);
		void setImageWidth(float width);

		void setTexture(const Texture* tex);

		void setMVP(const glm::mat4& mvp);

	protected:
		UniformTex image;
		Uniform transform;
		Uniform windowWidth;
		Uniform windowHeight;
	};

	class GaussianBlurVerticalShader : public Pass
	{
	public:
		GaussianBlurVerticalShader();

		virtual ~GaussianBlurVerticalShader() = default;

		void setImageHeight(float height);
		void setImageWidth(float width);

		void setTexture(const Texture* tex);

		void setMVP(const glm::mat4& mvp);

	protected:
		UniformTex image;
		Uniform transform;
		Uniform windowWidth;
		Uniform windowHeight;
	};
}