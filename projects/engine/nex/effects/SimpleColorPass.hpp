#pragma once

#include <nex/shader/Pass.hpp>
#include <nex/shader/Technique.hpp>

namespace nex
{
	class SimpleColorPass : public TransformPass
	{
	public:
		SimpleColorPass();

		void setColor(const glm::vec4 color);

	private:
		glm::vec4 mColor;
		Uniform mColorUniform;
	};

	class SimpleColorTechnique : public Technique 
	{
	public:
		SimpleColorTechnique();

		SimpleColorPass* getSimpleColorPass();

	private:
		SimpleColorPass mSimpleColorPass;
	};
}