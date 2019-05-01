#pragma once
#include <memory>
#include "nex/shader/Pass.hpp"
#include "nex/gui/Drawable.hpp"
#include "nex/mesh/VertexArray.hpp"

namespace nex
{
	class TesselationTest
	{
	public:
		TesselationTest();

		void draw(Camera* camera);

		class TesselationPass : public Pass
		{
		public:
			TesselationPass();

			void setUniforms(Camera* camera);

			unsigned outerLevel0Val;
			unsigned outerLevel1Val;
			unsigned outerLevel2Val;
			unsigned outerLevel3Val;
			unsigned innerLevel0Val;
			unsigned innerLevel1Val;

			Uniform outerLevel0;
			Uniform outerLevel1;
			Uniform outerLevel2;
			Uniform outerLevel3;
			Uniform innerLevel0;
			Uniform innerLevel1;

			Uniform transform;

			glm::mat4 mWorldTrafo;

		};

		std::unique_ptr<TesselationPass> mPass;
		std::unique_ptr<VertexArray> mMesh;
		std::unique_ptr<VertexBuffer> mBuffer;

	};


	namespace gui
	{
		class TesselationTest_Config : public nex::gui::Drawable
		{
		public:
			TesselationTest_Config(TesselationTest* tesselationTest);

		protected:
			void drawSelf() override;

			TesselationTest* mTesselationTest;
		};
	}
}
