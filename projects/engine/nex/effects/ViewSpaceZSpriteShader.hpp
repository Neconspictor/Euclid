#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/effects/SpriteShader.hpp>

namespace nex
{
	class ViewSpaceZSpriteShader : public SpriteShader
	{
	public:
		ViewSpaceZSpriteShader();

		void setCameraDistanceRange(float distanceRange);

		void update(const Texture* texture, const glm::mat4& mat) override;

		virtual ~ViewSpaceZSpriteShader() = default;

	protected:
		float mDistanceRange;
		Uniform mDistanceRangeUniform;
	};
}