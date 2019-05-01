#include <pbr_deferred/TesselationTest.hpp>
#include "nex/mesh/StaticMeshManager.hpp"
#include "nex/renderer/RenderBackend.hpp"
#include "nex/gui/Controller.hpp"
#include "nex/mesh/VertexLayout.hpp"
#include "HeightMap.hpp"

nex::TesselationTest::TesselationTest() : mPass(std::make_unique<TesselationPass>())
{
	mMesh = std::make_unique<VertexArray>();

	static const float fullscreenPlaneTriangleStripVerticesOpengl2[] = {
		// position 4 floats, texture coords 2 floats
		-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
		 0.0, -1.0, 0.0, 1.0, 0.0, 0.0,
		 0.0,  0.0, 0.0, 1.0, 0.0, 0.0,
		-1.0,  0.0, 0.0, 1.0, 0.0, 0.0,
		 
		-1.0,  0.0, 0.0, 1.0, 0.0, 0.0,
		 0.0,  0.0, 0.0, 1.0, 0.0, 0.0,
		 0.0,  1.0, 0.0, 1.0, 0.0, 0.0,
		-1.0,  1.0, 0.0, 1.0, 0.0, 0.0,
		 
		 0.0,  0.0, 0.0, 1.0, 0.0, 0.0,
		 1.0,  0.0, 0.0, 1.0, 0.0, 0.0,
		 1.0,  1.0, 0.0, 1.0, 0.0, 0.0,
		 0.0,  1.0, 0.0, 1.0, 0.0, 0.0,
		 
		 0.0, -1.0, 0.0, 1.0, 0.0, 0.0,
		 1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
		 1.0,  0.0, 0.0, 1.0, 0.0, 0.0,
		 0.0,  0.0, 0.0, 1.0, 0.0, 0.0,
	};

	mBuffer = std::make_unique<VertexBuffer>(fullscreenPlaneTriangleStripVerticesOpengl2, sizeof(fullscreenPlaneTriangleStripVerticesOpengl2));
	VertexLayout layout;
	layout.push<float>(4);
	layout.push<float>(2);
	mMesh->bind();
	mMesh->useBuffer(*mBuffer, layout);
	mMesh->unbind(); // important: In OpenGL implementation VertexBuffer creation with arguments corrupts state of vertex array, if not unbounded!

	HeightMap heightMap(4, 4,1,1,1);
}

void nex::TesselationTest::draw()
{
	mPass->bind();
	mPass->setUniforms();

	mMesh->bind();
	RenderBackend::get()->setPatchVertexCount(4);
	RenderState state;
	state.doBlend = false;
	state.doDepthTest = false;
	state.doCullFaces = true;
	state.fillMode = FillMode::LINE;

	// Only draw the first triangle
	RenderBackend::get()->drawArray(state, Topology::PATCHES, 0, 16);
	//RenderBackend::get()->drawArray(state, Topology::TRIANGLE_STRIP, 0, 3);
}

nex::TesselationTest::TesselationPass::TesselationPass() : Pass(Shader::create("test/tesselation/quads/tesselation_quads_vs.glsl", 
	"test/tesselation/quads/tesselation_quads_fs.glsl",
	"test/tesselation/quads/tesselation_quads_tcs.glsl",
	"test/tesselation/quads/tesselation_quads_tes.glsl"))
{

	outerLevel0 = { mShader->getUniformLocation("outerLevel0"), UniformType::UINT };
	outerLevel1 = { mShader->getUniformLocation("outerLevel1"), UniformType::UINT };
	outerLevel2 = { mShader->getUniformLocation("outerLevel2"), UniformType::UINT };
	outerLevel3 = { mShader->getUniformLocation("outerLevel3"), UniformType::UINT };
	innerLevel0 = { mShader->getUniformLocation("innerLevel0"), UniformType::UINT };
	innerLevel1 = { mShader->getUniformLocation("innerLevel1"), UniformType::UINT };

	outerLevel0Val = 8;
	outerLevel1Val = 8;
	outerLevel2Val = 8;
	outerLevel3Val = 8;
	innerLevel0Val = 8;
	innerLevel1Val = 8;
}

void nex::TesselationTest::TesselationPass::setUniforms()
{
	mShader->setUInt(outerLevel0.location, outerLevel0Val);
	mShader->setUInt(outerLevel1.location, outerLevel1Val);
	mShader->setUInt(outerLevel2.location, outerLevel2Val);
	mShader->setUInt(outerLevel3.location, outerLevel3Val);
	mShader->setUInt(innerLevel0.location, innerLevel0Val);
	mShader->setUInt(innerLevel1.location, innerLevel1Val);
}

nex::gui::TesselationTest_Config::TesselationTest_Config(TesselationTest* tesselationTest) : mTesselationTest(tesselationTest)
{
}

void nex::gui::TesselationTest_Config::drawSelf()
{
	// render configuration properties
	ImGui::PushID(m_id.c_str());

	auto max = RenderBackend::get()->getMaxPatchVertexCount();
	max = 8;

	ImGui::SliderInt("outer level 0", (int*)&mTesselationTest->mPass->outerLevel0Val, 1, max);
	ImGui::SliderInt("outer level 1", (int*)&mTesselationTest->mPass->outerLevel1Val, 1, max);
	ImGui::SliderInt("outer level 2", (int*)&mTesselationTest->mPass->outerLevel2Val, 1, max);
	ImGui::SliderInt("outer level 3", (int*)&mTesselationTest->mPass->outerLevel3Val, 1, max);
	ImGui::SliderInt("inner level 0", (int*)&mTesselationTest->mPass->innerLevel0Val, 0, max);
	ImGui::SliderInt("inner level 1", (int*)&mTesselationTest->mPass->innerLevel1Val, 0, max);

	ImGui::PopID();
}


//"test/tesselation/tesselation_tcs.glsl", 
//"test/tesselation/tesselation_tes.glsl"