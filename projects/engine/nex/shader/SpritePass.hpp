#pragma once
#include <nex/shader/Pass.hpp>

namespace nex
{
	class SpritePass : public Pass
	{
	public:
		SpritePass();

		void setTexture(const Texture* texture);
		void setTransform(const glm::mat4& mat);
	protected:
		UniformTex mTexture;
		Uniform mTransform;
	};
}