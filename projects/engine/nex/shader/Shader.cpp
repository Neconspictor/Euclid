#include <nex/shader/Shader.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <nex/camera/Camera.hpp>

nex::Shader::Shader(std::unique_ptr<ShaderProgram> program) : mProgram(std::move(program))
{
}

nex::Shader::~Shader() = default;

void nex::Shader::updateConstants(const RenderContext& constants)
{
}

void nex::Shader::updateInstance(const nex::RenderContext& constants, const glm::mat4& modelMatrix, const glm::mat4& prevModelMatrix, const void* data)
{
}

void nex::Shader::updateMaterial(const Material& material)
{
}

void nex::Shader::bind()
{
	mProgram->bind();
}

void nex::Shader::bindBoneTrafoBuffer(ShaderStorageBuffer* buffer) const
{
	buffer->bindToTarget(DEFAULT_BONE_BUFFER_BINDING_POINT);
}

nex::ShaderProgram* nex::Shader::getShader()
{
	return mProgram.get();
}

bool nex::Shader::isBound() const
{
	return mProgram->isBound();
}

void nex::Shader::setProgram(std::unique_ptr<ShaderProgram> shader)
{
	mProgram = std::move(shader);
}

void nex::Shader::unbind()
{
	mProgram->unbind();
}

nex::TransformShader::TransformShader(std::unique_ptr<ShaderProgram> program) : Shader(std::move(program))
{
}

nex::TransformShader::~TransformShader() = default;

void nex::TransformShader::setViewProjectionMatrices(const glm::mat4& projection, 
	const glm::mat4& view, 
	const glm::mat4& invView,
	const glm::mat4& prevView, 
	const glm::mat4& prevViewProj)
{
	mProjection = projection;
	mView = view;
	mPrevView = prevView;
	mPrevViewProjection = prevViewProj;
}

void nex::TransformShader::uploadTransformMatrices(
	const RenderContext& context,
	const glm::mat4& model, 
	const glm::mat4& prevModel)
{
	bind();
	
	auto& perObjectData = context.perObjectData;

	perObjectData.model = model;
	perObjectData.modelView = mView * perObjectData.model;
	perObjectData.transform = mProjection * perObjectData.modelView;
	perObjectData.prevTransform = mPrevViewProjection * prevModel;
	perObjectData.normalMatrix = glm::inverseTranspose(perObjectData.modelView);
	perObjectData.materialID = 0;
	//perObjectData.normalMatrix = glm::inverseTranspose(perObjectData.model);

	context.perObjectDataBuffer->resize(sizeof(PerObjectData), &perObjectData, nex::GpuBuffer::UsageHint::STREAM_DRAW); //nex::GpuBuffer::UsageHint::STREAM_DRAW
}

void nex::TransformShader::updateConstants(const nex::RenderContext& constants)
{
	auto* cam = constants.camera;
	setViewProjectionMatrices(cam->getProjectionMatrix(), cam->getView(), cam->getViewInv(), cam->getViewPrev(), cam->getViewProjPrev());
}

void nex::TransformShader::updateInstance(const RenderContext& constants, 
	const glm::mat4& modelMatrix, 
	const glm::mat4& prevModelMatrix, 
	const void* data)
{
	uploadTransformMatrices(constants, modelMatrix, prevModelMatrix);
}

nex::SimpleTransformShader::SimpleTransformShader(std::unique_ptr<ShaderProgram> program, unsigned transformLocation) : Shader(std::move(program)), mTransformLocation(transformLocation)
{
}

nex::SimpleTransformShader::~SimpleTransformShader() = default;

void nex::SimpleTransformShader::updateTransformMatrix(const glm::mat4& model)
{
	auto transform = mViewProjection * model;
	mProgram->bind();
	mProgram->setMat4(mTransformLocation, transform);
}

void nex::SimpleTransformShader::updateViewProjection(const glm::mat4& projection, const glm::mat4& view)
{
	mViewProjection = projection * view;
}

void nex::SimpleTransformShader::updateConstants(const RenderContext& constants)
{
	auto&  cam = *constants.camera;
	updateViewProjection(cam.getProjectionMatrix(), cam.getView());
}

void nex::SimpleTransformShader::updateInstance(const glm::mat4& model, const glm::mat4& prevModel)
{
	updateTransformMatrix(model);
}

nex::ComputeShader::ComputeShader(std::unique_ptr<ShaderProgram> program) : Shader(std::move(program))
{
}

nex::ComputeShader::~ComputeShader() = default;