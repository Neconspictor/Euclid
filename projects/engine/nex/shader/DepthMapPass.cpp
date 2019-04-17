#include <nex/shader/DepthMapPass.hpp>
#include <glm/glm.hpp>
#include <nex/texture/Texture.hpp>


nex::DepthMapPass::DepthMapPass()
{
	mShader = Shader::create(
		"depth_map_vs.glsl", "depth_map_fs.glsl");

	mDephTexture = { mShader->getUniformLocation("depthMap"), UniformType::TEXTURE2D, 0};

	mShader->setBinding(mDephTexture.location, mDephTexture.bindingSlot);

	auto state = mSampler.getState();
	state.wrapR = state.wrapS = state.wrapT = TextureUVTechnique::ClampToBorder;
	state.borderColor = glm::vec4(1.0);
	mSampler.setState(state);
}

void nex::DepthMapPass::useDepthMapTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mDephTexture.bindingSlot);
}