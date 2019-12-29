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

void nex::Shader::updateInstance(const glm::mat4& modelMatrix, const glm::mat4& prevModelMatrix, const void* data)
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

nex::TransformShader::TransformShader(std::unique_ptr<ShaderProgram> program, unsigned transformBindingPoint) : Shader(std::move(program)), mTransformBindingPoint(transformBindingPoint),
mTransformBuffer(mTransformBindingPoint, sizeof(Transforms), nullptr, ShaderBuffer::UsageHint::DYNAMIC_DRAW)
{
}

nex::TransformShader::~TransformShader() = default;

void nex::TransformShader::setViewProjectionMatrices(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& prevView, const glm::mat4& prevViewProj)
{
	mTransforms.projection = projection;
	mTransforms.view = view;
	mPrevView = prevView;
	mPrevViewProjection = prevViewProj;
}

void nex::TransformShader::setModelMatrix(const glm::mat4& model, const glm::mat4& prevModel)
{
	mTransforms.model = model;
	mPrevModel = prevModel;

}

void nex::TransformShader::uploadTransformMatrices()
{
	bind();
	mTransforms.modelView = mTransforms.view * mTransforms.model;
	mTransforms.transform = mTransforms.projection * mTransforms.modelView;
	mTransforms.prevTransform = mPrevViewProjection * mPrevModel;
	mTransforms.normalMatrix = glm::inverseTranspose(mTransforms.modelView);

	mTransformBuffer.bindToTarget();
	mTransformBuffer.update(sizeof(Transforms), &mTransforms);
	//RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderStorage);
}

void nex::TransformShader::updateConstants(const nex::RenderContext& constants)
{
	auto* cam = constants.camera;
	setViewProjectionMatrices(cam->getProjectionMatrix(), cam->getView(), cam->getViewPrev(), cam->getViewProjPrev());
}

void nex::TransformShader::updateInstance(const glm::mat4& modelMatrix, const glm::mat4& prevModelMatrix, const void* data)
{
	setModelMatrix(modelMatrix, prevModelMatrix);
	uploadTransformMatrices();
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