#include <nex/post_processing/SSR.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>

class nex::SSR::SSRComputeUVPass : public nex::Shader {
public:
	SSRComputeUVPass() : Shader(ShaderProgram::create("screen_space_vs.glsl", "post_processing/ssr_compute_uv_fs.glsl")) {
		mDepth = mProgram->createTextureUniform("depthMap", UniformType::TEXTURE2D, 0);
		mNormal = mProgram->createTextureUniform("normalMap", UniformType::TEXTURE2D, 1);
		mColor = mProgram->createTextureUniform("colorMap", UniformType::TEXTURE2D, 2);
		mInvProj = {mProgram->getUniformLocation("invProj"), UniformType::MAT4};
		mProj = { mProgram->getUniformLocation("proj"), UniformType::MAT4 };
		mClipInfo = { mProgram->getUniformLocation("clipInfo"), UniformType::VEC4 };
		
	}

	void setDepth(Texture* depth) {
		mProgram->setTexture(depth, Sampler::getPoint(), mDepth.bindingSlot);
	}

	void setNormal(Texture* normal) {
		mProgram->setTexture(normal, Sampler::getPoint(), mNormal.bindingSlot);
	}

	void setColor(Texture* color) {
		mProgram->setTexture(color, Sampler::getPoint(), mColor.bindingSlot);
	}

	void setInvProj(const glm::mat4& invProj) {
		mProgram->setMat4(mInvProj.location, invProj);
	}

	void setProj(const glm::mat4& proj) {
		mProgram->setMat4(mProj.location, proj);
	}

	void setClipInfo(const glm::vec4& clipInfo) {
		mProgram->setVec4(mClipInfo.location, clipInfo);
	}

private:
	UniformTex mDepth;
	UniformTex mNormal;
	UniformTex mColor;
	Uniform mInvProj;
	Uniform mProj;
	Uniform mClipInfo;
};

nex::SSR::SSR() :
	mSSRComputeUVPass(std::make_unique<SSRComputeUVPass>())
{
}

nex::SSR::~SSR() = default;

void nex::SSR::renderReflections(Texture* depth, Texture* normalsVS, 
	Texture* color,
	const glm::mat4& proj, const glm::mat4& invProj, 
	const glm::vec4& clipInfo)
{
	mRenderTarget->bind();
	RenderBackend::get()->setViewPort(0,0, mRenderTarget->getWidth(), mRenderTarget->getHeight());
	mRenderTarget->clear(RenderComponent::Color);

	mSSRComputeUVPass->bind();
	mSSRComputeUVPass->setDepth(depth);
	mSSRComputeUVPass->setNormal(normalsVS);
	mSSRComputeUVPass->setColor(color);
	mSSRComputeUVPass->setInvProj(invProj);
	mSSRComputeUVPass->setProj(proj);
	mSSRComputeUVPass->setClipInfo(clipInfo);

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
	desc.colorspace = ColorSpace::RGBA;
	desc.internalFormat = InternalFormat::RGBA16F;
	
	mRenderTarget = std::make_unique<RenderTarget2D>(width, height, desc);
}