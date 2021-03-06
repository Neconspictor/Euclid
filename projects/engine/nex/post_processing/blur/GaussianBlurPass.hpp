#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{

	class Sampler;

	class GaussianBlurHorizontalShader : public Shader
	{
	public:
		GaussianBlurHorizontalShader();

		virtual ~GaussianBlurHorizontalShader() = default;

		void setImageHeight(float height);
		void setImageWidth(float width);

		void setTexture(const Texture* tex);

	protected:
		UniformTex image;
		Uniform windowWidth;
		Uniform windowHeight;
	};

	class GaussianBlurVerticalShader : public Shader
	{
	public:
		GaussianBlurVerticalShader();

		virtual ~GaussianBlurVerticalShader() = default;

		void setImageHeight(float height);
		void setImageWidth(float width);

		void setTexture(const Texture* tex);

	protected:
		UniformTex image;
		Uniform windowWidth;
		Uniform windowHeight;
	};
}