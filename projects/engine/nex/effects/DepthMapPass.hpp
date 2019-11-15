#pragma once
#include <nex/shader/Shader.hpp>

namespace nex
{
	class DepthMapPass : public SimpleTransformShader
	{
	public:
		DepthMapPass();

		virtual ~DepthMapPass() = default;
	};
}