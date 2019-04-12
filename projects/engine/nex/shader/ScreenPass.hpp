#pragma once
#include <nex/shader/Pass.hpp>

namespace nex
{
	class ScreenPass : public TransformPass
	{
	public:
		ScreenPass();

		void useTexture(const Texture* texture);

		void setMVP(const glm::mat4& mat);

		void onTransformUpdate(const TransformData& data) override;
	protected:
		UniformTex mTexture;
		Uniform mTransform;
	};
}