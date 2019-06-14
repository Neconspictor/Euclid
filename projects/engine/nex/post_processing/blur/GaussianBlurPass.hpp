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

		void setImageHeight(Real height);
		void setImageWidth(Real width);

		void setTexture(const Texture* tex);

	protected:
		UniformTex image;
		Uniform windowWidth;
		Uniform windowHeight;
	};

	class GaussianBlurVerticalShader : public Pass
	{
	public:
		GaussianBlurVerticalShader();

		virtual ~GaussianBlurVerticalShader() = default;

		void setImageHeight(Real height);
		void setImageWidth(Real width);

		void setTexture(const Texture* tex);

	protected:
		UniformTex image;
		Uniform windowWidth;
		Uniform windowHeight;
	};
}