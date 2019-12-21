#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{
	class SpriteShader : public Shader
	{
	public:
		SpriteShader();

		SpriteShader(std::unique_ptr<ShaderProgram> program);

		virtual ~SpriteShader();

		/**
		 * Note: this shader has to be bound!
		 */
		virtual void update(const Texture* texture, const glm::mat4& mat);
	protected:
		UniformTex mTexture;
		Uniform mTransform;
	};

	class DepthSpriteShader : public SpriteShader
	{
	public:
		DepthSpriteShader();
		virtual ~DepthSpriteShader();
	};
}