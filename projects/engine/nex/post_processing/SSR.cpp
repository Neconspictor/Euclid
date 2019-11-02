#include <nex/post_processing/SSR.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>

class nex::SSR::SSRComputeUVPass : public nex::Pass {
public:
	SSRComputeUVPass() : Pass(Shader::create("screen_space_vs.glsl", "post_processing/ssr_compute_uv_fs.glsl")) {
		mDepth = mShader->createTextureUniform("depthMap", UniformType::TEXTURE2D, 0);
		mNormal = mShader->createTextureUniform("normalMap", UniformType::TEXTURE2D, 1);
		mInvProj = {mShader->getUniformLocation("invProj"), UniformType::MAT4};
		mProj = { mShader->getUniformLocation("proj"), UniformType::MAT4 };
	}

	void setDepth(Texture* depth) {
		mShader->setTexture(depth, Sampler::getPoint(), mDepth.bindingSlot);
	}

	void setNormal(Texture* normal) {
		mShader->setTexture(normal, Sampler::getPoint(), mNormal.bindingSlot);
	}

	void setInvProj(const glm::mat4& invProj) {
		mShader->setMat4(mInvProj.location, invProj);
	}

	void setProj(const glm::mat4& proj) {
		mShader->setMat4(mProj.location, proj);
	}

private:
	UniformTex mDepth;
	UniformTex mNormal;
	Uniform mInvProj;
	Uniform mProj;
};

nex::SSR::SSR(unsigned width, unsigned height) :
	mSSRComputeUVPass(std::make_unique<SSRComputeUVPass>())
{
	resize(width, height);
}

void nex::SSR::renderReflections(Texture* depth, Texture* normalsVS, const glm::mat4& proj, const glm::mat4& invProj)
{
	mRenderTarget->bind();
	RenderBackend::get()->setViewPort(0,0, mRenderTarget->getWidth(), mRenderTarget->getHeight());
	mRenderTarget->clear(RenderComponent::Color);

	mSSRComputeUVPass->bind();
	mSSRComputeUVPass->setDepth(depth);
	mSSRComputeUVPass->setNormal(normalsVS);
	mSSRComputeUVPass->setInvProj(invProj);
	mSSRComputeUVPass->setProj(proj);

	StaticMeshDrawer::drawFullscreenTriangle(RenderState::getNoDepthTest(), mSSRComputeUVPass.get());
}

nex::Texture* nex::SSR::getReflectionUV()
{
	return mRenderTarget->getColorAttachmentTexture(0);
}

const nex::Texture* nex::SSR::getReflectionUV() const
{
	return mRenderTarget->getColorAttachmentTexture(0);
}

void nex::SSR::resize(unsigned width, unsigned height)
{
	TextureDesc desc;
	desc.generateMipMaps = false;
	desc.colorspace = ColorSpace::RGB;
	desc.internalFormat = InternalFormat::RGB16;
	
	mRenderTarget = std::make_unique<RenderTarget2D>(width, height, desc);
}