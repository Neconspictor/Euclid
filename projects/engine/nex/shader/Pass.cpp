#include <nex/shader/Pass.hpp>

nex::Pass::Pass(std::unique_ptr<Shader> program) : mShader(std::move(program))
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

void nex::Pass::onModelMatrixUpdate(const glm::mat4& modelMatrix)
{
}

void nex::Pass::onMaterialUpdate(const Material* material)
{
}

void nex::Pass::setupRenderState()
{
}

void nex::Pass::reverseRenderState()
{
}

nex::TransformPass::TransformPass(std::unique_ptr<Shader> program) : Pass(std::move(program))
{
}

nex::ComputePass::ComputePass(std::unique_ptr<Shader> program) : Pass(std::move(program))
{
}