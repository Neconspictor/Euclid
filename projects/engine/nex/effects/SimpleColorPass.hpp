#pragma once

#include <nex/shader/Shader.hpp>

namespace nex
{
	class SimpleColorPass : public TransformShader
	{
	public:
		SimpleColorPass();

		void setColor(const glm::vec4 color);

		void updateMaterial(const Material& material) override;

	private:
		glm::vec4 mColor;
		Uniform mColorUniform;
	};
}