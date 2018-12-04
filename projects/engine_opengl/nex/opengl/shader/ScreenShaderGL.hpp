#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{
	class ScreenShader : public TransformShaderGL
	{
	public:
		ScreenShader();

		void useTexture(const Texture* texture);

		void setMVP(const glm::mat4& mat);

		void onTransformUpdate(const TransformData& data) override;
	protected:
		UniformTex mTexture;
		Uniform mTransform;
	};
}