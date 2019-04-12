#pragma once
#include <nex/shader/Pass.hpp>

namespace nex
{
	class ScreenShader : public TransformPass
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