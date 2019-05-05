#pragma once
#include <memory>
#include "nex/shader/Pass.hpp"
#include "nex/gui/Drawable.hpp"
#include "nex/mesh/VertexArray.hpp"
#include "HeightMap.hpp"

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

			void setUniforms(Camera* camera, const glm::mat4& trafo);

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
		};

		class NormalPass : public Pass
		{
		public:
			NormalPass();
			void setUniforms(Camera* camera, const TesselationPass& transformPass, const glm::mat4& trafo);

			Uniform modelViewUniform;
			Uniform projectionUniform;
			Uniform transformUniform;
			Uniform normalMatrixUniform;
			Uniform colorUniform;

			Uniform outerLevel0;
			Uniform outerLevel1;
			Uniform outerLevel2;
			Uniform outerLevel3;
			Uniform innerLevel0;
			Uniform innerLevel1;
		};

		std::unique_ptr<TesselationPass> mPass;
		std::unique_ptr<NormalPass> mNormalPass;
		std::unique_ptr<VertexArray> mMesh;
		std::unique_ptr<VertexBuffer> mBuffer;

		glm::mat4 mWorldTrafo;

		HeightMap mHeightMap;

		bool mShowNormals;
		bool mWireframe;
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
