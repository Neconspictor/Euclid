#include <pbr_deferred/TesselationTest.hpp>
#include "nex/mesh/StaticMeshManager.hpp"
#include "nex/renderer/RenderBackend.hpp"

nex::TesselationTest::TesselationTest() : mPass(std::make_unique<TesselationPass>())
{
	mMesh = StaticMeshManager::get()->getNDCFullscreenPlane();

}

void nex::TesselationTest::draw()
{
	mPass->bind();
	mMesh->bind();
	RenderBackend::get()->setPatchVertexCount(3);
	RenderState state;
	state.doBlend = false;
	state.doDepthTest = false;
	state.doCullFaces = false;
	state.fillMode = FillMode::LINE;

	// Only draw the first triangle
	//RenderBackend::get()->drawArray(state, Topology::PATCHES, 0, 3);
	RenderBackend::get()->drawArray(state, Topology::TRIANGLE_STRIP, 0, 3);
}

nex::TesselationTest::TesselationPass::TesselationPass() : Pass(Shader::create("test/tesselation/tesselation_vs.glsl", 
	"test/tesselation/tesselation_fs.glsl",
	"test/tesselation/tesselation_tcs.glsl",
	"test/tesselation/tesselation_tes.glsl"))
{
}


//"test/tesselation/tesselation_tcs.glsl", 
//"test/tesselation/tesselation_tes.glsl"