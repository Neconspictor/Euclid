#pragma once
#include <memory>
#include "nex/shader/Pass.hpp"

namespace nex
{
	class VertexArray;


	class TesselationTest
	{
	public:
		TesselationTest();

		void draw();

	private:

		class TesselationPass : public Pass
		{
		public:
			TesselationPass();
		};

		std::unique_ptr<TesselationPass> mPass;
		VertexArray* mMesh;

	};
}
