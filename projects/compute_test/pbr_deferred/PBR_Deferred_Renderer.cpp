#include <pbr_deferred/PBR_Deferred_Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/camera/TrackballQuatCamera.hpp>
#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <nex/opengl/scene/SceneNode.hpp>
#include <nex/opengl/shader/ScreenShaderGL.hpp>
#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/opengl/shading_model/ShadingModelFactoryGL.hpp>
#include "nex/opengl/model/ModelManagerGL.hpp"
#include <nex/opengl/shader/ShaderGL.hpp>
#include "nex/opengl/texture/TextureGL.hpp"
#include <math.h>
#include<algorithm>
#include "nex/util/Timer.hpp"
#include <ostream>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/string_cast.hpp>

namespace nex {
	class Timer;
}

using namespace glm;
using namespace std;
using namespace nex;

int ssaaSamples = 1;


nex::PBR_Deferred_Renderer::ComputeTestShader::ComputeTestShader(unsigned width, unsigned height) : ComputeShader()
{
	std::vector<UnresolvedShaderStageDesc> unresolved;
	unresolved.resize(1);

	unresolved[0].filePath = "test/compute/compute_test.glsl";
	unresolved[0].defines.push_back("#define PARTITIONS 6");
	unresolved[0].type = ShaderStageType::COMPUTE;

	ShaderSourceFileGenerator* generator = ShaderSourceFileGenerator::get();
	ProgramSources programSources = generator->generate(unresolved);

	std::vector<Guard<ShaderStage>> shaderStages;
	shaderStages.resize(programSources.descs.size());
	for (auto i = 0; i < shaderStages.size(); ++i)
	{
		auto path = generator->getFileSystem()->resolvePath("test/compute");
		path += "/compute_test-resolved.glsl";
		FileSystem::writeToFile(path.generic_string(), programSources.descs[i].root.resolvedSource);
		shaderStages[i] = ShaderStage::compileShaderStage(programSources.descs[i]);
	}

	mProgram = ShaderProgram::create(shaderStages);

	/*GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);*/

	TextureData tData;
	tData.internalFormat = InternFormat::R32UI;
	tData.colorspace = ColorSpace::RED_INTEGER;
	tData.pixelDataType = PixelDataType::UINT;
	tData.useSwizzle = false;
	tData.generateMipMaps = false;
	tData.wrapR = TextureUVTechnique::ClampToEdge;
	tData.wrapS = TextureUVTechnique::ClampToEdge;
	tData.wrapT = TextureUVTechnique::ClampToEdge;
	tData.magFilter = TextureFilter::NearestNeighbor;
	tData.minFilter = TextureFilter::NearestNeighbor;

	//result = new Texture(new TextureGL(textureID));

	unbind();

	uniformBuffer = new ShaderStorageBufferGL(2, sizeof(ShaderBuffer), ShaderBufferGL::DYNAMIC_DRAW);

	storageBuffer = new ShaderStorageBufferGL(1, sizeof(WriteOut), ShaderBufferGL::DYNAMIC_COPY);


	//ShaderProgramGL* gl = (ShaderProgramGL*)mProgram->mImpl;
	//GLuint id = glGetUniformBlockIndex(gl->getProgramID(), "BufferData");

	bind();
	uniformBuffer->bind();
	Guard<ShaderBuffer> data; 
	data = new ShaderBuffer();
	data->mCameraNearFar = vec4(0.03, 1.0, 0.0, 0.0);
	data->mColor = vec4(1.0, 1.0, 1.0, 1.0);
	data->mCameraProj = glm::perspectiveFov<float>(radians(45.0f),
		ComputeTestShader::width,
		ComputeTestShader::height, 0.1f, 100.0f);

	data->mCameraViewToLightProj = glm::ortho(-82.802315f, 82.8023150f, -41.380932f, 41.380932f, 0.1f, 100.0f);
	//data->mCameraViewToLightProj  = glm::perspectiveFov<float>(radians(45.0f),
	//	ComputeTestShader::width,
	//	ComputeTestShader::height, 0.1f, 100.0f);

	const float step = 1.0f / (float)partitionCount;

	const float begin = -0.2f;
	const float end = -100.0f;

	for(unsigned i = 0; i < partitionCount; ++i)
	{
		auto& partition = data->partitions[i];
		partition.intervalBegin = begin + i * step * (end - begin);
		partition.intervalEnd = begin + (i+1) * step * (end - begin);
	}

	//data->partitions[0].intervalBegin = 0.00001f;

	uniformBuffer->update(data.get(), sizeof(*data));

	std::vector<float> memory(ComputeTestShader::width * ComputeTestShader::height);
	auto size = memory.size();


	

	srand(static_cast <unsigned> (time(0)));

	for (auto i = 0; i < size; ++i)
	{
		memory[i] = -0.0f;//static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

	memory[2048*1024 - 1] = 0.999;
	//memory[2048-1] = 0.0001;
	//memory[0] = 0.000009;
	//memory[1] = 0.2;
	//memory[2048] = 0.99999;

	vec3 viewSpaceResult(-82.802315, -41.380932, -100.000061);
	vec4 clipSpace = data->mCameraViewToLightProj * vec4(viewSpaceResult, 1.0f);
	vec3 texCoord = (vec3(clipSpace) / clipSpace.w)* 0.5f + vec3(0.5f);
	std::cout << "texCoord = " << glm::to_string(texCoord) << std::endl;

	vec3 viewSpaceResult2(-0.091992, -0.045974, -0.111099);
	vec4 clipSpace2 = data->mCameraViewToLightProj * vec4(viewSpaceResult2, 1.0f);
	vec3 texCoord2 = (vec3(clipSpace2) / clipSpace2.w)* 0.5f + vec3(0.5f);
	std::cout << "texCoord2 = " << glm::to_string(texCoord2) << std::endl;

	vec3 viewSpaceResult3(82.802315, 41.380932, -100.000061);
	vec4 clipSpace3 = data->mCameraViewToLightProj * vec4(viewSpaceResult3, 1.0f);
	vec3 texCoord3 = (vec3(clipSpace3) / clipSpace3.w)* 0.5f + vec3(0.5f);
	std::cout << "texCoord3 = " << glm::to_string(texCoord3) << std::endl;


	//memory[748373] = 0.07;
	//memory[748373] = -0.07;
	//memory[ComputeTestShader::width * ComputeTestShader::height - 1] = 0.99;

	//data->mDepthValues[7483] = 0.004;
	//data->mDepthValues[50000] = 0.002;
	//data->mDepthValues[650043] = 0.003;
	//memory[1000030] = 0.001;

	//auto minValue = std::min_element(memory.begin(), memory.end());

	//std::cout << "minValue = " << *minValue << std::endl;


	depth = Texture::createTexture2D(ComputeTestShader::width, ComputeTestShader::height, tData, memory.data());

	storageBuffer->bind();
	WriteOut dataOut;
	reset(&dataOut);
	storageBuffer->update(&dataOut, sizeof(dataOut));


	UniformLocation location = getProgram()->getUniformLocation("depthTexture");
	getProgram()->setImageLayerOfTexture(0,
		depth.get(),
		0,
		TextureAccess::READ_ONLY,
		InternFormat::R32UI,
		0,
		false,
		0);
}

void PBR_Deferred_Renderer::ComputeTestShader::reset(WriteOut* out)
{
	for (int i = 0; i < partitionCount; ++i)
	{
		out->results[i].minCoord = vec3(FLT_MAX);
		out->results[i].maxCoord = vec3(0.0f);
	}

	out->lock = 0;
}

PBR_Deferred_Renderer::ComputeClearColorShader::ComputeClearColorShader(Texture* texture) : ComputeShader()
{
	std::vector<UnresolvedShaderStageDesc> unresolved;
	unresolved.resize(1);

	unresolved[0].filePath = "test/compute/compute_clear_color.glsl";
	unresolved[0].type = ShaderStageType::COMPUTE;

	ShaderSourceFileGenerator* generator = ShaderSourceFileGenerator::get();
	ProgramSources programSources = generator->generate(unresolved);

	std::vector<Guard<ShaderStage>> shaderStages;
	shaderStages.resize(programSources.descs.size());
	for (auto i = 0; i < shaderStages.size(); ++i)
	{
		shaderStages[i] = ShaderStage::compileShaderStage(programSources.descs[i]);
	}

	mProgram = ShaderProgram::create(shaderStages);

	bind();

	UniformLocation location = getProgram()->getUniformLocation("data");
	getProgram()->setImageLayerOfTexture(location,
		texture,
		0,
		TextureAccess::READ_WRITE,
		InternFormat::RGBA32F,
		0,
		false,
		0);

	unbind();
}

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
PBR_Deferred_Renderer::PBR_Deferred_Renderer(RendererOpenGL* backend) :
	Renderer(backend),
	m_logger("PBR_Deferred_Renderer"),
	renderTargetSingleSampled(nullptr)
{
}

void PBR_Deferred_Renderer::init(int windowWidth, int windowHeight)
{
	using namespace placeholders;

	ModelManagerGL* modelManager = m_renderBackend->getModelManager();

	modelManager->loadModels();

	renderTargetSingleSampled = m_renderBackend->createRenderTarget();


	vec3 position = {1.0f, 1.0f, 1.0f };
	position = 15.0f * position;
	globalLight.setPosition(position);
	globalLight.lookAt({0,0,0});
	globalLight.update(true);
	globalLight.setColor(vec3(1.0f, 1.0f, 1.0f));


	vec2 dim = {1.0, 1.0};
	vec2 pos = {0, 0};

	// center
	pos.x = 0.5f * (1.0f - dim.x);
	pos.y = 0.5f * (1.0f - dim.y);

	// align to bottom corner
	//pos.x = 1.0f - dim.x;
	//pos.y = 1.0f - dim.y;

	//align to top right corner
	//pos.x = 1.0f - dim.x;
	//pos.y = 0;

	screenSprite.setPosition(pos);
	screenSprite.setWidth(dim.x);
	screenSprite.setHeight(dim.y);


	RenderTarget* screenRenderTarget = m_renderBackend->getDefaultRenderTarget();
	m_renderBackend->useBaseRenderTarget(screenRenderTarget);
	m_renderBackend->setViewPort(0, 0, windowWidth, windowHeight);
	m_renderBackend->clearRenderTarget(screenRenderTarget, RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

	mComputeTest = new ComputeTestShader(windowWidth, windowHeight);

	//mComputeTest->bind();
	//GLuint location = mComputeTest->getProgram()->getUniformLocation("data");
	//GLCall(glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGB8));
	//GLCall(glUniform1i(location, 0));
	//mComputeTest->getProgram()->setTexture(0, mComputeTest->result.get(), 0);
	//glBindImageTexture(0, HeightMap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

}

void PBR_Deferred_Renderer::render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight)
{
	//ModelDrawerGL* modelDrawer = m_renderBackend->getModelDrawer();
	//ScreenShader* screenShader = (ScreenShader*)(
	//	m_renderBackend->getShaderManager()->getShader(ShaderType::Screen));
	using namespace chrono;

	const unsigned width = mComputeTest->width;//mComputeTest->result->getWidth();
	const unsigned height = mComputeTest->height; // mComputeTest->result->getHeight();



	mComputeTest->bind();


	unsigned xDim = 16 * 16; // 256
	unsigned yDim = 8 * 8; // 128

	unsigned dispatchX = width % xDim == 0 ? width / xDim : width / xDim + 1;
	unsigned dispatchY = height % yDim == 0 ? height / yDim : height / yDim + 1;


	static Timer timer;
	static uint64 counter = 0;
	static uint64 sum = 0;
	static uint64 min = 10000000;
	static uint64 max = 0;
	timer.update();
	mComputeTest->dispatch(dispatchX, dispatchY, 1);

	ComputeTestShader::WriteOut* result = (ComputeTestShader::WriteOut*) mComputeTest->storageBuffer->map(ShaderBufferGL::READ_WRITE);

	static bool printed = false;

	if (!printed)
	{
		std::cout << "result->lock = " << result->lock << "\n";

		for (int i = 0; i < mComputeTest->partitionCount; ++i)
		{
			std::cout << "result->minResult[" << i << "] = " << glm::to_string(result->results[i].minCoord) << "\n";
			std::cout << "result->maxResult[" << i << "] = " << glm::to_string(result->results[i].maxCoord) << std::endl;
		}

		printed = true;
	}

	// reset
	mComputeTest->reset(result);
	mComputeTest->storageBuffer->unmap();

	timer.update();


	const auto diff = timer.getTimeInMicros();
	sum += diff;
	++counter;

	if (diff < min) min = diff;
	if (diff > max)
	{
		if (diff < 2000) max = diff;
	}	

	if (diff > 1000 && false)
	{
		std::cout << "Diff: " << diff << " ; AVG = " << sum / counter << 
			 " ; Min = " << min << " ; Max = " << max << std::endl;
	}
}

void PBR_Deferred_Renderer::updateRenderTargets(int width, int height)
{
	//update render target dimension
	//the render target dimensions are dependent from the viewport size
	// so first update the viewport and than recreate the render targets
	m_renderBackend->resize(width, height);
	m_renderBackend->destroyRenderTarget(renderTargetSingleSampled);
	renderTargetSingleSampled = m_renderBackend->createRenderTarget();

	/*

	TextureData tData;
	tData.internalFormat = InternFormat::R32F;
	tData.colorspace = ColorSpace::R;
	tData.useSwizzle = false;
	tData.generateMipMaps = false;
	tData.wrapR = TextureUVTechnique::ClampToEdge;
	tData.wrapS = TextureUVTechnique::ClampToEdge;
	tData.wrapT = TextureUVTechnique::ClampToEdge;
	tData.magFilter = TextureFilter::NearestNeighbor;
	tData.minFilter = TextureFilter::NearestNeighbor;

	mComputeTest->depth = Texture::createTexture2D(width, height, tData, nullptr);


	mComputeTest->bind();

	mComputeTest->getProgram()->setImageLayerOfTexture(0,
		mComputeTest->depth.get(),
		0,
		TextureAccess::READ_ONLY,
		InternFormat::R32UI,
		0,
		false,
		0);

	*/
}

PBR_Deferred_Renderer_ConfigurationView::PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer) : m_renderer(renderer)
{
}

void PBR_Deferred_Renderer_ConfigurationView::drawSelf()
{
	// render configuration properties
	ImGui::PushID(m_id.c_str());
	ImGui::PopID();
}