#pragma once
#include <nex/shader/Pass.hpp>

namespace nex
{
	class SpritePass : public Pass
	{
	public:
		SpritePass();

		SpritePass(std::unique_ptr<Shader> shader);

		virtual ~SpritePass();

		void setTexture(const Texture* texture);
		void setTransform(const glm::mat4& mat);
	protected:
		UniformTex mTexture;
		Uniform mTransform;
	};

	class DepthSpritePass : public SpritePass
	{
	public:
		DepthSpritePass();
		virtual ~DepthSpritePass();
	};
}