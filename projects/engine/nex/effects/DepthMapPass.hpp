#pragma once
#include <nex/shader/Pass.hpp>

namespace nex
{
	class DepthMapPass : public SimpleTransformPass
	{
	public:
		DepthMapPass();

		virtual ~DepthMapPass() = default;
	};
}