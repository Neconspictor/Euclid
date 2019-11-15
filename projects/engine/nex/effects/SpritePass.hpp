#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{
	class SpritePass : public Shader
	{
	public:
		SpritePass();

		SpritePass(std::unique_ptr<ShaderProgram> shader);

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