#include <techniques/TesselationTest.hpp>
#include "nex/mesh/StaticMeshManager.hpp"
#include "nex/renderer/RenderBackend.hpp"
#include "nex/gui/Controller.hpp"
#include "nex/mesh/VertexLayout.hpp"
#include "HeightMap.hpp"
#include <nex/camera/Camera.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

nex::TesselationTest::TesselationTest() : mPass(std::make_unique<TesselationPass>()), mNormalPass(std::make_unique<NormalPass>()), mHeightMap(std::move(HeightMap::createRandom(10,10, 2, 0.4f,  2)))
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

	static const float fullscreenPlaneTriangleStripVerticesOpengl3[] = {
		// position 4 floats, texture coords 2 floats
		-1.0, -1.0, 0.0, 
		 0.0, -1.0, 0.0,
		 0.0,  0.0, 0.0, 

		-1.0, -1.0, 0.0, 
		 0.0,  0.0, 0.0, 
		-1.0,  0.0, 0.0,

		-1.0,  0.0, 0.0,
		 0.0,  0.0, 0.0, 
		 0.0,  1.0, 0.0, 

		-1.0,  0.0, 0.0, 
		 0.0,  1.0, 0.0,  
		-1.0,  1.0, 0.0, 

		 0.0,  0.0, 0.0, 
		 1.0,  0.0, 0.0,  
		 1.0,  1.0, 0.0, 
		 

		0.0,  0.0, 0.0, 
		1.0,  1.0, 0.0, 
		0.0,  1.0, 0.0,

		 0.0, -1.0, 0.0,
		 1.0, -1.0, 0.0,
		 1.0,  0.0, 0.0,

		0.0, -1.0, 0.0,
		1.0,  0.0, 0.0,
		0.0,  0.0, 0.0,
	};

	//mBuffer = std::make_unique<VertexBuffer>(fullscreenPlaneTriangleStripVerticesOpengl2, sizeof(fullscreenPlaneTriangleStripVerticesOpengl2));
	mBuffer = std::make_unique<VertexBuffer>(sizeof(fullscreenPlaneTriangleStripVerticesOpengl2), fullscreenPlaneTriangleStripVerticesOpengl2);
	VertexLayout layout;
	layout.push<float>(4, mBuffer.get());
	layout.push<float>(2, mBuffer.get());
	mMesh->bind();
	mMesh->init(layout);
	mMesh->unbind(); // important: In OpenGL implementation VertexBuffer creation with arguments corrupts state of vertex array, if not unbounded!

	const glm::mat4 unit(1.0f);
	//auto translate = unit;
	glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 2.0f, 0.0f));
	//auto scale = glm::mat4();
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));

	mWorldTrafo = translateMatrix;

	mWireframe = false;
	mShowNormals = false;
}

void nex::TesselationTest::draw(Camera* camera, const glm::vec3& lightDir)
{
	mPass->bind();
	mPass->setUniforms(camera, mWorldTrafo, &mHeightMap, lightDir);

	//mMesh->bind();
	auto* mesh = mHeightMap.getMesh();
	mesh->getVertexArray().bind();
	mesh->getIndexBuffer().bind();
	RenderBackend::get()->setPatchVertexCount(4);
	RenderState state;
	state.doBlend = false;
	state.doDepthTest = true;
	state.doDepthWrite = true;
	state.doCullFaces = false;

	if (mWireframe)
	{
		state.fillMode = FillMode::LINE;
	} else
	{
		state.fillMode = FillMode::FILL;
	}
	

	state.depthCompare = CompareFunction::LESS;

	// Only draw the first triangle
	RenderBackend::get()->drawWithIndices(state, Topology::PATCHES, mesh->getIndexBuffer().getCount(), mesh->getIndexBuffer().getType());


	if (mShowNormals)
	{
		RenderBackend::get()->setLineThickness(5.0f);
		mNormalPass->bind();
		mNormalPass->setUniforms(camera, *mPass, mWorldTrafo, &mHeightMap);
		state.doCullFaces = false;
		//state.doDepthTest = false;
		//state.doDepthWrite = false;
		RenderBackend::get()->drawWithIndices(state, Topology::PATCHES, mesh->getIndexBuffer().getCount(), mesh->getIndexBuffer().getType());
	}
}

nex::TesselationTest::TesselationPass::TesselationPass() : Pass(Shader::create("test/tesselation/heightmap/tesselation_heightmap_vs.glsl", 
	"test/tesselation/heightmap/tesselation_heightmap_fs.glsl",
	"test/tesselation/heightmap/tesselation_heightmap_tcs.glsl",
	"test/tesselation/heightmap/tesselation_heightmap_tes.glsl"))
{

	outerLevel0 = { mShader->getUniformLocation("outerLevel0"), UniformType::UINT };
	outerLevel1 = { mShader->getUniformLocation("outerLevel1"), UniformType::UINT };
	outerLevel2 = { mShader->getUniformLocation("outerLevel2"), UniformType::UINT };
	outerLevel3 = { mShader->getUniformLocation("outerLevel3"), UniformType::UINT };
	innerLevel0 = { mShader->getUniformLocation("innerLevel0"), UniformType::UINT };
	innerLevel1 = { mShader->getUniformLocation("innerLevel1"), UniformType::UINT };

	transform = {mShader->getUniformLocation("transform"), UniformType::MAT4};
	heightMap = { mShader->getUniformLocation("heightMap"), UniformType::TEXTURE2D, 0};
	worldDimensionUniform = { mShader->getUniformLocation("worldDimension"), UniformType::VEC3 };
	lightUniform = { mShader->getUniformLocation("lightDirViewSpace"), UniformType::VEC3 };
	normalMatrixUniform = { mShader->getUniformLocation("normalMatrix"), UniformType::MAT3 };
	modelViewUniform = { mShader->getUniformLocation("modelView"), UniformType::MAT4 };
	segmentCountUniform = { mShader->getUniformLocation("segmentCount"), UniformType::VEC2 };

	outerLevel0Val = 2;
	outerLevel1Val = 2;
	outerLevel2Val = 2;
	outerLevel3Val = 2;
	innerLevel0Val = 2;
	innerLevel1Val = 2;
}

void nex::TesselationTest::TesselationPass::setUniforms(Camera* camera, const glm::mat4& trafo, HeightMap* heightMap, const glm::vec3 lightDir)
{
	mShader->setUInt(outerLevel0.location, outerLevel0Val);
	mShader->setUInt(outerLevel1.location, outerLevel1Val);
	mShader->setUInt(outerLevel2.location, outerLevel2Val);
	mShader->setUInt(outerLevel3.location, outerLevel3Val);
	mShader->setUInt(innerLevel0.location, innerLevel0Val);
	mShader->setUInt(innerLevel1.location, innerLevel1Val);

	auto projection = camera->getProjectionMatrix();
	auto view = camera->getView();

	auto modelView = view * trafo;

	mShader->setMat4(modelViewUniform.location, modelView);
	mShader->setMat3(normalMatrixUniform.location, createNormalMatrix(modelView));

	mShader->setMat4(transform.location, projection * view * trafo);


	mShader->setTexture(heightMap->getHeightTexture(), heightMap->getHeightSampler(), this->heightMap.bindingSlot);
	mShader->setVec3(worldDimensionUniform.location, heightMap->getWorldDimension());


	glm::vec3 lightDirViewSpace = glm::vec3(view * glm::vec4(lightDir, 0.0));

	mShader->setVec3(lightUniform.location, normalize(lightDirViewSpace));
	mShader->setVec2(segmentCountUniform.location, glm::vec2(heightMap->getVertexCount().x - 1, heightMap->getVertexCount().y - 1));
}

nex::TesselationTest::NormalPass::NormalPass() : Pass(Shader::create("test/tesselation/heightmap/normals_vs.glsl",
	"test/tesselation/heightmap/normals_fs.glsl",
	"test/tesselation/heightmap/normals_tcs.glsl",
	"test/tesselation/heightmap/normals_tes.glsl",
	"test/tesselation/heightmap/normals_gs.glsl"))
{
	modelViewUniform = { mShader->getUniformLocation("modelView"), UniformType::MAT4 };
	projectionUniform = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	transformUniform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	normalMatrixUniform = { mShader->getUniformLocation("normalMatrix"), UniformType::MAT4 };
	colorUniform = { mShader->getUniformLocation("color"), UniformType::VEC4 };

	outerLevel0 = { mShader->getUniformLocation("outerLevel0"), UniformType::UINT };
	outerLevel1 = { mShader->getUniformLocation("outerLevel1"), UniformType::UINT };
	outerLevel2 = { mShader->getUniformLocation("outerLevel2"), UniformType::UINT };
	outerLevel3 = { mShader->getUniformLocation("outerLevel3"), UniformType::UINT };
	innerLevel0 = { mShader->getUniformLocation("innerLevel0"), UniformType::UINT };
	innerLevel1 = { mShader->getUniformLocation("innerLevel1"), UniformType::UINT };

	heightMap = { mShader->getUniformLocation("heightMap"), UniformType::TEXTURE2D, 0 };
	worldDimensionUniform = { mShader->getUniformLocation("worldDimension"), UniformType::VEC3 };
	segmentCountUniform = { mShader->getUniformLocation("segmentCount"), UniformType::VEC2 };
}

void nex::TesselationTest::NormalPass::setUniforms(Camera* camera, const TesselationPass& transformPass, const glm::mat4& trafo, HeightMap* heightMap)
{
	const auto& projection = camera->getProjectionMatrix();
	const auto& view = camera->getView();

	auto transform = projection * view * trafo;

	auto modelView = view * trafo;



	mShader->setMat4(modelViewUniform.location, modelView);
	mShader->setMat4(projectionUniform.location, projection);
	mShader->setMat4(transformUniform.location, transform);
	mShader->setMat3(normalMatrixUniform.location, createNormalMatrix(modelView));
	mShader->setVec4(colorUniform.location, {0,0,1,1});

	mShader->setUInt(outerLevel0.location, transformPass.outerLevel0Val);
	mShader->setUInt(outerLevel1.location, transformPass.outerLevel1Val);
	mShader->setUInt(outerLevel2.location, transformPass.outerLevel2Val);
	mShader->setUInt(outerLevel3.location, transformPass.outerLevel3Val);
	mShader->setUInt(innerLevel0.location, transformPass.innerLevel0Val);
	mShader->setUInt(innerLevel1.location, transformPass.innerLevel1Val);

	mShader->setTexture(heightMap->getHeightTexture(), heightMap->getHeightSampler(), this->heightMap.bindingSlot);
	mShader->setVec3(worldDimensionUniform.location, heightMap->getWorldDimension());
	mShader->setVec2(segmentCountUniform.location, glm::vec2(heightMap->getVertexCount().x - 1, heightMap->getVertexCount().y - 1));
}

nex::gui::TesselationTest_Config::TesselationTest_Config(TesselationTest* tesselationTest) : mTesselationTest(tesselationTest)
{
}

void nex::gui::TesselationTest_Config::drawSelf()
{
	// render configuration properties
	ImGui::PushID(mId.c_str());

	auto max = RenderBackend::get()->getMaxPatchVertexCount();
	max = 8;

	ImGui::SliderInt("outer level 0", (int*)&mTesselationTest->mPass->outerLevel0Val, 1, max);
	ImGui::SliderInt("outer level 1", (int*)&mTesselationTest->mPass->outerLevel1Val, 1, max);
	ImGui::SliderInt("outer level 2", (int*)&mTesselationTest->mPass->outerLevel2Val, 1, max);
	ImGui::SliderInt("outer level 3", (int*)&mTesselationTest->mPass->outerLevel3Val, 1, max);
	ImGui::SliderInt("inner level 0", (int*)&mTesselationTest->mPass->innerLevel0Val, 0, max);
	ImGui::SliderInt("inner level 1", (int*)&mTesselationTest->mPass->innerLevel1Val, 0, max);

	ImGui::Checkbox("Show normals", &mTesselationTest->mShowNormals);
	ImGui::Checkbox("Wireframe", &mTesselationTest->mWireframe);

	ImGui::PopID();
}