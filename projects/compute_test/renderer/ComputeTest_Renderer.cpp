#include <renderer/ComputeTest_Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/camera/TrackballQuatCamera.hpp>
#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <nex/opengl/scene/SceneNode.hpp>
#include <nex/opengl/shading_model/ShadingModelFactoryGL.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/util/Timer.hpp>
#include <ostream>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/string_cast.hpp>
#include <iostream>

namespace nex {
	class Timer;
}

using namespace glm;
using namespace std;
using namespace nex;

int ssaaSamples = 1;


nex::ComputeTest_Renderer::ComputeTestShader::ComputeTestShader(unsigned width, unsigned height) : ComputeShader()
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
	for (unsigned i = 0; i < shaderStages.size(); ++i)
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

	uniformBuffer = new ShaderStorageBuffer(2, sizeof(Constant), ShaderBuffer::UsageHint::DYNAMIC_DRAW);

	storageBuffer = new ShaderStorageBuffer(1, sizeof(WriteOut), ShaderBuffer::UsageHint::DYNAMIC_COPY);

	lockBuffer = new ShaderStorageBuffer(3, sizeof(LockBuffer), ShaderBuffer::UsageHint::DYNAMIC_COPY);


	//ShaderProgramGL* gl = (ShaderProgramGL*)mProgram->mImpl;
	//GLuint id = glGetUniformBlockIndex(gl->getProgramID(), "BufferData");

	bind();


	lockBuffer->bind();

	LockBuffer buffer;
	buffer.lock = 0;
	lockBuffer->update(&buffer, sizeof(LockBuffer));

	lockBuffer->unbind();

	uniformBuffer->bind();
	Guard<Constant> data;
	data = new Constant();
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

	const float begin = -0.1f;
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
		memory[i] = 0.0f;//static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

	//memory[2048*1024 - 1] = 1.0; //vec3(0.250408, 0.125143, 0.302416)
	memory[2048-1] = 0.99999999;
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


	//vec3(0.206696, 0.103297, 0.249626)
	vec3 viewSpaceResult4 (0.206696, -0.103297, -0.249626);
	vec4 clipSpace4 = data->mCameraViewToLightProj * vec4(viewSpaceResult4, 1.0f);
	vec3 texCoord4 = (vec3(clipSpace4) / clipSpace4.w)* 0.5f + vec3(0.5f);
	std::cout << "texCoord4 = " << glm::to_string(texCoord4) << std::endl;


	//memory[748373] = 0.07;
	//memory[748373] = -0.07;
	//memory[ComputeTestShader::width * ComputeTestShader::height - 1] = 0.99;

	//data->mDepthValues[7483] = 0.004;
	//data->mDepthValues[50000] = 0.002;
	//data->mDepthValues[650043] = 0.003;
	//memory[1000030] = 0.001;

	//auto minValue = std::min_element(memory.begin(), memory.end());

	//std::cout << "minValue = " << *minValue << std::endl;


	depth = Texture2D::create(ComputeTestShader::width, ComputeTestShader::height, tData, memory.data());

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

void ComputeTest_Renderer::ComputeTestShader::setDepthTexture(Texture* depth, InternFormat format)
{
	getProgram()->setImageLayerOfTexture(0,
		depth,
		0,
		TextureAccess::READ_ONLY,
		format,
		0,
		false,
		0);
}

void ComputeTest_Renderer::ComputeTestShader::reset(WriteOut* out)
{
	for (int i = 0; i < partitionCount; ++i)
	{
		out->results[i].minCoord = vec3(FLT_MAX);
		out->results[i].maxCoord = vec3(0.0f);
	}

	//out->lock = 0;
}

ComputeTest_Renderer::ComputeClearColorShader::ComputeClearColorShader(Texture* texture) : ComputeShader()
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

ComputeTest_Renderer::SimpleBlinnPhong::SimpleBlinnPhong() : mView(nullptr), mProjection(nullptr)
{
	mProgram = ShaderProgram::create("test/compute/blinn_phong_simple_vs.glsl",
		"test/compute/blinn_phong_simple_fs.glsl");

	mTransformMatrix = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mModelMatrix = { mProgram->getUniformLocation("model"), UniformType::MAT4 };
	mViewPositionWorld = { mProgram->getUniformLocation("viewPos_world"), UniformType::MAT4 };
	mDirLightDirection = { mProgram->getUniformLocation("dirLight.direction"), UniformType::MAT4 };
}

void nex::ComputeTest_Renderer::SimpleBlinnPhong::setLightDirection(const glm::vec3 lightDirectionWorld)
{
	mProgram->setVec3(mDirLightDirection.location, lightDirectionWorld);
}

void ComputeTest_Renderer::SimpleBlinnPhong::setView(const glm::mat4* view)
{
	mView = view;
}

void ComputeTest_Renderer::SimpleBlinnPhong::setProjection(const glm::mat4* projection)
{
	mProjection = projection;
}

void ComputeTest_Renderer::SimpleBlinnPhong::setViewPositionWorld(const glm::vec3 viewPositionWorld)
{
	mProgram->setVec3(mViewPositionWorld.location, viewPositionWorld);
}

void ComputeTest_Renderer::SimpleBlinnPhong::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	mat4 modelView = *mView * modelMatrix;

	const mat4 transform = *mProjection * *mView * modelMatrix;

	mProgram->setMat4(mTransformMatrix.location, transform);
	mProgram->setMat4(mModelMatrix.location, modelMatrix);
}

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
ComputeTest_Renderer::ComputeTest_Renderer(RendererOpenGL* backend) :
	Renderer(backend),
	m_logger("PBR_Deferred_Renderer"),
	renderTargetSingleSampled(nullptr)
{
}

void ComputeTest_Renderer::init(int windowWidth, int windowHeight)
{
	using namespace placeholders;

	ModelManagerGL* modelManager = m_renderBackend->getModelManager();

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

	mSimpleBlinnPhong = make_unique<SimpleBlinnPhong>();

	//mComputeTest->bind();
	//GLuint location = mComputeTest->getProgram()->getUniformLocation("data");
	//GLCall(glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGB8));
	//GLCall(glUniform1i(location, 0));
	//mComputeTest->getProgram()->setTexture(0, mComputeTest->result.get(), 0);
	//glBindImageTexture(0, HeightMap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

}

void ComputeTest_Renderer::render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight)
{
	ModelDrawerGL* modelDrawer = m_renderBackend->getModelDrawer();
	//ScreenShader* screenShader = (ScreenShader*)(
	//	m_renderBackend->getShaderManager()->getShader(ShaderType::Screen));
	using namespace chrono;

	const unsigned width = mComputeTest->width;//mComputeTest->result->getWidth();
	const unsigned height = mComputeTest->height; // mComputeTest->result->getHeight();

	RenderTarget* screenRenderTarget = m_renderBackend->getDefaultRenderTarget();
	m_renderBackend->useBaseRenderTarget(screenRenderTarget);
	m_renderBackend->setViewPort(0, 0, windowWidth, windowHeight);
	m_renderBackend->clearRenderTarget(screenRenderTarget, RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);


	mSimpleBlinnPhong->bind();

	mSimpleBlinnPhong->setLightDirection(vec3(100, 100, 100));
	mSimpleBlinnPhong->setViewPositionWorld(camera->getPosition());
	mSimpleBlinnPhong->setView(&camera->getView());
	mSimpleBlinnPhong->setProjection(&camera->getPerspProjection());
	modelDrawer->draw(scene, mSimpleBlinnPhong.get());

	auto depthStencilMap = screenRenderTarget->getDepthStencilMap();


	return;



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

	ComputeTestShader::WriteOut* result = (ComputeTestShader::WriteOut*) mComputeTest->storageBuffer->map(ShaderBuffer::Access::READ_WRITE);

	static bool printed = false;

	if (!printed)
	{
		//std::cout << "result->lock = " << result->lock << "\n";

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

void ComputeTest_Renderer::updateRenderTargets(int width, int height)
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