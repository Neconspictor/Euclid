#include <nex/water/PSSR.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/renderer/RenderBackend.hpp>

class nex::PSSR::ProjHashPass : public nex::ComputePass 
{
public:
	ProjHashPass() : ComputePass(Shader::createComputeShader("TODO")) 
	{
		mProjHashTexture = mShader->createTextureUniform("projHashMap", UniformType::IMAGE2D, 0);
		mColorTexture = mShader->createTextureUniform("colorMap", UniformType::TEXTURE2D, 0);

		mViewProjMatrix = {mShader->getUniformLocation("viewProj"), UniformType::MAT4};
		mInvViewProjMatrix = { mShader->getUniformLocation("invViewProj"), UniformType::MAT4 };
	}


	void setViewProj(const glm::mat4& mat) {
		mShader->setMat4(mViewProjMatrix.location, mat);
	}

	void setInvViewProj(const glm::mat4& mat) {
		mShader->setMat4(mInvViewProjMatrix.location, mat);
	}

	void setProjHashTexture(Texture* texture) {
		mShader->setImageLayerOfTexture(mProjHashTexture.location,
			texture,
			mProjHashTexture.bindingSlot,
			TextureAccess::WRITE_ONLY,
			InternalFormat::R32UI,
			0,
			false,
			0);
	}

	void setColorTexture(Texture* texture) {
		mShader->setTexture(texture, Sampler::getPoint(), mColorTexture.bindingSlot);
	}

private:
	UniformTex mProjHashTexture;
	UniformTex mColorTexture;

	Uniform mViewProjMatrix;
	Uniform mInvViewProjMatrix;
};


nex::PSSR::~PSSR() = default;

nex::Texture2D* nex::PSSR::getProjHashBuffer()
{
	return mProjHasBuffer.get();
}

void nex::PSSR::renderProjectionHash(Texture* color, const glm::mat4& viewProj, const glm::mat4& invViewProj)
{
	mProjHashPass->bind();
	mProjHashPass->setColorTexture(color);
	mProjHashPass->setProjHashTexture(mProjHasBuffer.get());
	mProjHashPass->setViewProj(viewProj);
	mProjHashPass->setInvViewProj(invViewProj);

	mProjHashPass->dispatch(color->getWidth(), color->getHeight(), 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess | MemorySync_TextureFetch);
}

void nex::PSSR::resize(unsigned width, unsigned height)
{
	TextureDesc desc;
	desc.colorspace = ColorSpace::RED_INTEGER;
	desc.internalFormat = InternalFormat::R32UI;
	desc.generateMipMaps = false;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.magFilter = desc.minFilter = TexFilter::Nearest;
	desc.wrapR = desc.wrapS = desc.wrapT = UVTechnique::ClampToEdge;

	mProjHasBuffer = std::make_unique<Texture2D>(width, height, desc, nullptr);
}