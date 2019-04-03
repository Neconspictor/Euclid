#include <renderer/ComputeTest_Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/camera/TrackballQuatCamera.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/util/Timer.hpp>
#include <ostream>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "nex/shader/DepthMapShader.hpp"
#include "nex/shader_generator/ShaderSourceFileGenerator.hpp"
#include "nex/RenderBackend.hpp"

namespace nex {
	class Timer;
}

using namespace glm;
using namespace std;
using namespace nex;

int ssaaSamples = 1;


void ComputeTest_Renderer::ComputeTestShader::setConstants(float viewNearZ, float viewFarZ, const glm::mat4& projection, const glm::mat4& cameraViewToLightProjection)
{
	uniformBuffer->bind();
	Guard<Constant> data;
	data = new Constant();
	data->mCameraNearFar = vec4(0.03, 1.0, 0.0, 0.0);
	data->mColor = vec4(1.0, 1.0, 1.0, 1.0);
	data->mCameraProj = projection;

	data->mCameraViewToLightProj = cameraViewToLightProjection;

	const float step = 1.0f / (float)partitionCount;

	const float begin = viewNearZ;
	const float end = viewFarZ;

	for (unsigned i = 0; i < partitionCount; ++i)
	{
		auto& partition = data->partitions[i];
		partition.intervalBegin = begin + i * step * (end - begin);
		partition.intervalEnd = begin + (i + 1) * step * (end - begin);
	}

	//data->partitions[0].intervalBegin = 0.00001f;

	uniformBuffer->update(data.get(), sizeof(*data));
}

nex::ComputeTest_Renderer::ComputeTestShader::ComputeTestShader(unsigned width, unsigned height) : ComputeShader()
{
	std::vector<UnresolvedShaderStageDesc> unresolved;
	unresolved.resize(1);

	unresolved[0].filePath = "test/compute/compute_test.glsl";

	std::stringstream ss;
	ss << "#define PARTITIONS " << partitionCount;
	unresolved[0].defines.push_back(ss.str());
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


	auto projection = glm::perspectiveFov<float>(radians(45.0f),
		ComputeTestShader::width,
		ComputeTestShader::height, 0.1f, 100.0f);

	auto viewToLightProjection = glm::ortho(-82.802315f, 82.8023150f, -41.380932f, 41.380932f, 0.1f, 100.0f);

	setConstants(-0.1, -100.0, projection, viewToLightProjection);

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

	/*vec3 viewSpaceResult(-82.802315, -41.380932, -100.000061);
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
	std::cout << "texCoord4 = " << glm::to_string(texCoord4) << std::endl;*/


	//memory[748373] = 0.07;
	//memory[748373] = -0.07;
	//memory[ComputeTestShader::width * ComputeTestShader::height - 1] = 0.99;

	//data->mDepthValues[7483] = 0.004;
	//data->mDepthValues[50000] = 0.002;
	//data->mDepthValues[650043] = 0.003;
	//memory[1000030] = 0.001;

	//auto minValue = std::min_element(memory.begin(), memory.end());

	//std::cout << "minValue = " << *minValue << std::endl;



	storageBuffer->bind();
	WriteOut dataOut;

	for (auto i = 0; i < partitionCount; ++i)
	{
		dataOut.results[i].minCoord = vec4(1.0f);
		dataOut.results[i].maxCoord = vec4(0.0f);
	}

	//reset(&dataOut);
	storageBuffer->update(&dataOut, sizeof(dataOut));

	ComputeTestShader::WriteOut* result = (ComputeTestShader::WriteOut*) storageBuffer->map(ShaderBuffer::Access::READ_WRITE);

	static bool printed = false;

	if (!printed)
	{
		//std::cout << "result->lock = " << result->lock << "\n";

		for (int i = 0; i < partitionCount; ++i)
		{
			std::cout << "result->minResult[" << i << "] = " << glm::to_string(result->results[i].minCoord) << "\n";
			std::cout << "result->maxResult[" << i << "] = " << glm::to_string(result->results[i].maxCoord) << std::endl;
		}

		printed = true;
	}

	// reset
	reset(result);
	storageBuffer->unmap();


	/*depth = Texture2D::create(ComputeTestShader::width, ComputeTestShader::height, tData, memory.data());
	UniformLocation location = getProgram()->getUniformLocation("depthTexture");
	getProgram()->setImageLayerOfTexture(0,
		depth.get(),
		0,
		TextureAccess::READ_ONLY,
		InternFormat::R32UI,
		0,
		false,
		0);*/
}

void ComputeTest_Renderer::ComputeTestShader::setDepthTexture(Texture* depth, InternFormat format)
{
	UniformLocation location = getProgram()->getUniformLocation("depthTexture");
	getProgram()->setTexture(location, depth, 0);
	/*getProgram()->setImageLayerOfTexture(0,
		depth,
		0,
		TextureAccess::READ_ONLY,
		format,
		0,
		false,
		0);*/
}

void ComputeTest_Renderer::ComputeTestShader::reset(WriteOut* out)
{
	for (int i = 0; i < partitionCount; ++i)
	{
		out->results[i].minCoord = vec4(FLT_MAX);
		out->results[i].maxCoord = vec4(0.0f);
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



ComputeTest_Renderer::SimpleGeometryShader::SimpleGeometryShader() : mProjection(nullptr), mView(nullptr)
{
	mProgram = ShaderProgram::create("test/compute/simpl_geometry_vs.glsl",
		"test/compute/simpl_geometry_fs.glsl");

	mTransformMatrix = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

void ComputeTest_Renderer::SimpleGeometryShader::onModelMatrixUpdate(const glm::mat4& modelMatrix)
{
	const mat4 transform = *mProjection * *mView * modelMatrix;
	mProgram->setMat4(mTransformMatrix.location, transform);
}

void ComputeTest_Renderer::SimpleGeometryShader::setView(const glm::mat4* view)
{
	mView = view;
}

void ComputeTest_Renderer::SimpleGeometryShader::setProjection(const glm::mat4* projection)
{
	mProjection = projection;
}

ComputeTest_Renderer::GBuffer::GBuffer(unsigned width, unsigned height) : RenderTarget(), mDepth(nullptr)
{
	bind();

	TextureData data;
	data.minFilter = TextureFilter::NearestNeighbor;
	data.magFilter = TextureFilter::NearestNeighbor;
	data.wrapR = TextureUVTechnique::ClampToEdge;
	data.wrapS = TextureUVTechnique::ClampToEdge;
	data.wrapT = TextureUVTechnique::ClampToEdge;
	data.generateMipMaps = false;
	data.useSwizzle = false;

	RenderAttachment temp;

	// depth
	data.colorspace = ColorSpace::R;
	data.internalFormat = InternFormat::R32F;
	data.pixelDataType = PixelDataType::FLOAT;
	temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
	temp.target = TextureTarget::TEXTURE2D;
	temp.colorAttachIndex = 0;
	mDepth = temp.texture.get();
	addColorAttachment(temp);


	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	updateColorAttachment(0);

	// create and attach depth buffer (renderbuffer)
	// depth/stencil
	TextureData desc;
	desc.minFilter = TextureFilter::NearestNeighbor;
	desc.magFilter = TextureFilter::NearestNeighbor;
	desc.wrapS = TextureUVTechnique::ClampToEdge;
	desc.wrapT = TextureUVTechnique::ClampToEdge;
	desc.internalFormat = InternFormat::DEPTH24_STENCIL8;
	desc.colorspace = ColorSpace::DEPTH_STENCIL;
	desc.pixelDataType = PixelDataType::UNSIGNED_INT_24_8;

	temp.texture = make_shared<Texture2D>(width, height, desc, nullptr);
	temp.type = RenderAttachment::Type::DEPTH_STENCIL;
	temp.target = TextureTarget::TEXTURE2D;
	useDepthAttachment(temp);

	// finally check if framebuffer is complete
	if (!isComplete())
		throw_with_trace(std::runtime_error("ComputeTest_Renderer::GBuffer::GBuffer: Couldn't successfully init framebuffer!"));

	unbind();
}

Texture* ComputeTest_Renderer::GBuffer::getDepth() const
{
	return mDepth;
}

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
ComputeTest_Renderer::ComputeTest_Renderer(RenderBackend* backend, Input* input) :
	Renderer(backend),
	m_logger("PBR_Deferred_Renderer"),
	renderTargetSingleSampled(nullptr),
	mInput(input)
{
}

void ComputeTest_Renderer::init(int windowWidth, int windowHeight)
{
	using namespace placeholders;

	static auto* meshManager = StaticMeshManager::get();

	renderTargetSingleSampled = m_renderBackend->createRenderTarget();


	vec3 position = {1.0f, 1.0f, 1.0f };
	globalLight.setDirection(-position);
	globalLight.setColor(vec3(1.0f, 1.0f, 1.0f));


	vec2 dim = {1.0, 1.0};
	vec2 pos = {0, 0};

	// center
	pos.x = 0.5f * (1.0f - dim.x);
	pos.y = 0.5f * (1.0f - dim.y);

	screenSprite.setPosition(pos);
	screenSprite.setWidth(dim.x);
	screenSprite.setHeight(dim.y);


	RenderTarget* screenRenderTarget = m_renderBackend->getDefaultRenderTarget();
	screenRenderTarget->bind();
	m_renderBackend->setViewPort(0, 0, windowWidth, windowHeight);
	screenRenderTarget->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);


	mGBuffer = make_unique<GBuffer>(windowWidth, windowHeight);

	mComputeTest = make_unique<ComputeTestShader>(windowWidth, windowHeight);

	mComputeTest->bind();
	mComputeTest->setDepthTexture(mGBuffer->getDepth(), InternFormat::R32F);

	mSimpleBlinnPhong = make_unique<SimpleBlinnPhong>();

	mSimpleGeometry = make_unique<SimpleGeometryShader>();

	mSceneNearFarComputeShader = make_unique<SceneNearFarComputeShader>();


	m_renderBackend->getRasterizer()->enableScissorTest(false);

}

void ComputeTest_Renderer::render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight)
{
	renderNew(scene, camera, frameTime, windowWidth, windowHeight);
	//renderOld(scene, camera, frameTime, windowWidth, windowHeight);
}

void ComputeTest_Renderer::renderNew(SceneNode* scene, Camera* camera, float frameTime, int windowWidth,
	int windowHeight)
{
	using namespace chrono;

	const unsigned width = windowWidth;
	const unsigned height = windowHeight;

	m_renderBackend->setViewPort(0, 0, windowWidth, windowHeight);

	mGBuffer->bind();
	mGBuffer->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

	mSimpleGeometry->bind();
	mSimpleGeometry->setView(&camera->getView());
	mSimpleGeometry->setProjection(&camera->getPerspProjection());

	StaticMeshDrawer::draw(scene, mSimpleGeometry.get());

	RenderTarget* screenRenderTarget = m_renderBackend->getDefaultRenderTarget();
	screenRenderTarget->bind();
	screenRenderTarget->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

	/*depthMapShader->bind();
	depthMapShader->useDepthMapTexture(depthBuffer);

	modelDrawer->draw(&screenSprite, depthMapShader);*/

	//return;

	mSimpleBlinnPhong->bind();

	mSimpleBlinnPhong->setLightDirection(vec3(100, 100, 100));
	mSimpleBlinnPhong->setViewPositionWorld(camera->getPosition());
	mSimpleBlinnPhong->setView(&camera->getView());
	mSimpleBlinnPhong->setProjection(&camera->getPerspProjection());
	StaticMeshDrawer::draw(scene, mSimpleBlinnPhong.get());

	//auto depthStencilMap = screenRenderTarget->getDepthStencilMap();




	mSceneNearFarComputeShader->bind();


	unsigned xDim = 16 * 16; // 256
	unsigned yDim = 8 * 8; // 128

	unsigned dispatchX = width % xDim == 0 ? width / xDim : width / xDim + 1;
	unsigned dispatchY = height % yDim == 0 ? height / yDim : height / yDim + 1;

	const auto frustum = camera->getFrustum(Perspective);

	mSceneNearFarComputeShader->setConstants(frustum.nearPlane + 0.05, frustum.farPlane - 0.05, camera->getPerspProjection());
	mSceneNearFarComputeShader->setDepthTexture(mGBuffer->getDepth());

	mSceneNearFarComputeShader->dispatch(dispatchX, dispatchY, 1);

	auto result = mSceneNearFarComputeShader->readResult();

	static bool printed = false;

	if (!printed || mInput->isPressed(Input::KEY_K))
	{
		//std::cout << "result->lock = " << result->lock << "\n";

		std::cout << "----------------------------------------------------------------------------\n";
			std::cout << "result min/max = " << glm::to_string(glm::vec2(result.minMax)) << "\n";
		std::cout << "----------------------------------------------------------------------------\n" << std::endl;

		printed = true;
	}

	// reset
	mSceneNearFarComputeShader->reset();
}

void ComputeTest_Renderer::renderOld(SceneNode* scene, Camera* camera, float frameTime, int windowWidth,
	int windowHeight)
{
	using namespace chrono;

	const unsigned width = windowWidth;
	const unsigned height = windowHeight;

	m_renderBackend->setViewPort(0, 0, windowWidth, windowHeight);

	mGBuffer->bind();
	mGBuffer->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

	mSimpleGeometry->bind();
	mSimpleGeometry->setView(&camera->getView());
	mSimpleGeometry->setProjection(&camera->getPerspProjection());

	StaticMeshDrawer::draw(scene, mSimpleGeometry.get());

	RenderTarget* screenRenderTarget = m_renderBackend->getDefaultRenderTarget();
	screenRenderTarget->bind();
	screenRenderTarget->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);

	/*depthMapShader->bind();
	depthMapShader->useDepthMapTexture(depthBuffer);

	modelDrawer->draw(&screenSprite, depthMapShader);*/

	//return;

	mSimpleBlinnPhong->bind();

	mSimpleBlinnPhong->setLightDirection(vec3(100, 100, 100));
	mSimpleBlinnPhong->setViewPositionWorld(camera->getPosition());
	mSimpleBlinnPhong->setView(&camera->getView());
	mSimpleBlinnPhong->setProjection(&camera->getPerspProjection());
	StaticMeshDrawer::draw(scene, mSimpleBlinnPhong.get());

	//auto depthStencilMap = screenRenderTarget->getDepthStencilMap();




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



	auto projection = glm::perspectiveFov<float>(radians(45.0f),
		width,
		height, 0.1f, 100.0f);

	auto viewToLightProjection = glm::ortho(-82.802315f, 82.8023150f, -41.380932f, 41.380932f, 0.1f, 100.0f);

	mat4 transform = camera->getView();
	const vec3 look = camera->getLook();
	vec4 nearPos = vec4(camera->getPosition() + (0.1f*look), 1.0);
	nearPos = transform * nearPos;
	vec4 farPos = vec4(camera->getPosition() + (100.f*look), 1.0f);
	farPos = transform * farPos;

	mComputeTest->setConstants(nearPos.z, farPos.z, projection, viewToLightProjection);
	mComputeTest->setDepthTexture(mGBuffer->getDepth(), InternFormat::R32F);

	mComputeTest->dispatch(dispatchX, dispatchY, 1);

	mComputeTest->storageBuffer->bind();
	ComputeTestShader::WriteOut* result = (ComputeTestShader::WriteOut*) mComputeTest->storageBuffer->map(ShaderBuffer::Access::READ_WRITE);

	static bool printed = false;

	if (!printed || mInput->isPressed(Input::KEY_K))
	{
		//std::cout << "result->lock = " << result->lock << "\n";

		std::cout << "----------------------------------------------------------------------------\n";

		for (int i = 0; i < mComputeTest->partitionCount; ++i)
		{
			std::cout << "result->minResult[" << i << "] = " << glm::to_string(glm::vec3(result->results[i].minCoord)) << "\n";
			std::cout << "result->maxResult[" << i << "] = " << glm::to_string(glm::vec3(result->results[i].maxCoord)) << "\n";
		}

		std::cout << "----------------------------------------------------------------------------\n" << std::endl;

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
	renderTargetSingleSampled = m_renderBackend->createRenderTarget();
	mGBuffer = make_unique<GBuffer>(width, height);

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
