#include <nex/shader/Pass.hpp>
#include "ShaderBuffer.hpp"

nex::Pass::Pass(std::unique_ptr<Shader> program) : mShader(std::move(program))
{
}

nex::Pass::~Pass() = default;

void nex::Pass::updateConstants(Camera* camera)
{
}

void nex::Pass::bind()
{
	mShader->bind();
}

nex::Shader* nex::Pass::getShader()
{
	return mShader.get();
}

void nex::Pass::setShader(std::unique_ptr<Shader> shader)
{
	mShader = std::move(shader);
}

void nex::Pass::unbind()
{
	mShader->unbind();
}

void nex::Pass::setupRenderState()
{
}

void nex::Pass::reverseRenderState()
{
}

nex::TransformPass::TransformPass(std::unique_ptr<Shader> program) : Pass(std::move(program)),
mTransformBuffer(0, sizeof(Transforms), ShaderBuffer::UsageHint::DYNAMIC_DRAW)
{
}

nex::TransformPass::~TransformPass() = default;

void nex::TransformPass::setViewProjectionMatrices(const glm::mat4* projection, const glm::mat4* view)
{
	mTransforms.projection = *projection;
	mTransforms.view = *view;
}

void nex::TransformPass::setModelMatrix(const glm::mat4* model)
{
	mTransforms.model = *model;
}

void nex::TransformPass::uploadTransformMatrices()
{
	bind();
	mTransforms.modelView = mTransforms.view * mTransforms.model;
	mTransforms.transform = mTransforms.projection * mTransforms.modelView;
	mTransforms.normalMatrix = inverse(transpose(glm::mat3(mTransforms.modelView)));
	mTransformBuffer.bind();
	mTransformBuffer.update(&mTransforms, sizeof(Transforms));
}

nex::ComputePass::ComputePass(std::unique_ptr<Shader> program) : Pass(std::move(program))
{
}

nex::ComputePass::~ComputePass() = default;