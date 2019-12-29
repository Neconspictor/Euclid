#include <nex/water/PSSR.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/renderer/RenderBackend.hpp>

class nex::PSSR::ProjHashPass : public nex::ComputeShader 
{
public:
	ProjHashPass() : ComputeShader(ShaderProgram::createComputeShader("water/projection_hash_cs.glsl")) 
	{
		mProjHashTexture = mProgram->createTextureUniform("projHashMap", UniformType::IMAGE2D, 0);
		mDepthTexture = mProgram->createTextureUniform("depthMap", UniformType::TEXTURE2D, 0);

		mViewProjMatrix = {mProgram->getUniformLocation("viewProj"), UniformType::MAT4};
		mInvViewProjMatrix = { mProgram->getUniformLocation("invViewProj"), UniformType::MAT4 };
		mTexSize = { mProgram->getUniformLocation("texSize"), UniformType::VEC2 };
		mWaterHeight = { mProgram->getUniformLocation("waterHeight"), UniformType::FLOAT };
		mCameraDirection = { mProgram->getUniformLocation("cameraDirection"), UniformType::VEC3 };
	}


	void setViewProj(const glm::mat4& mat) {
		mProgram->setMat4(mViewProjMatrix.location, mat);
	}

	void setInvViewProj(const glm::mat4& mat) {
		mProgram->setMat4(mInvViewProjMatrix.location, mat);
	}

	void setProjHashTexture(Texture* texture) {
		mProgram->setImageLayerOfTexture(mProjHashTexture.location,
			texture,
			mProjHashTexture.bindingSlot,
			TextureAccess::WRITE_ONLY,
			InternalFormat::R32UI,
			0,
			false,
			0);
	}

	void setDepthTexture(const Texture* texture) {
		mProgram->setTexture(texture, Sampler::getPoint(), mDepthTexture.bindingSlot);
	}

	void setTexSize(const glm::vec2& texSize) {
		mProgram->setVec2(mTexSize.location, texSize);
	}

	void setWaterHeight(float height) {
		mProgram->setFloat(mWaterHeight.location, height);
	}

	void setCameraDir(const glm::vec3& vec) {
		mProgram->setVec3(mCameraDirection.location, vec);
	}

private:
	UniformTex mProjHashTexture;
	UniformTex mDepthTexture;

	Uniform mViewProjMatrix;
	Uniform mInvViewProjMatrix;
	Uniform mTexSize;
	Uniform mWaterHeight;
	Uniform mCameraDirection;
};


class nex::PSSR::ProjHashClearPass : public nex::ComputeShader
{
public:
	ProjHashClearPass() : ComputeShader(ShaderProgram::createComputeShader("water/projection_hash_clear_cs.glsl"))
	{
		mProjHashTexture = mProgram->createTextureUniform("projHashMap", UniformType::IMAGE2D, 0);
	}

	void setProjHashTexture(Texture* texture) {
		mProgram->setImageLayerOfTexture(mProjHashTexture.location,
			texture,
			mProjHashTexture.bindingSlot,
			TextureAccess::WRITE_ONLY,
			InternalFormat::R32UI,
			0,
			false,
			0);
	}

private:
	UniformTex mProjHashTexture;
};


nex::PSSR::PSSR(unsigned width, unsigned height) :
	mProjHashPass(std::make_unique<ProjHashPass>()),
	mProjHashClearPass(std::make_unique<ProjHashClearPass>())
{
	resize(width, height);
}

nex::PSSR::~PSSR() = default;

nex::Texture2D* nex::PSSR::getProjHashTexture()
{
	return mProjHashTexture.get();
}

void nex::PSSR::renderProjectionHash(const Texture* depth, const glm::mat4& viewProj, const glm::mat4& invViewProj, float waterHeight,
	const glm::vec3& cameraDir)
{
	glm::vec2 texSize(depth->getWidth(), depth->getHeight());

	mProjHashClearPass->bind();
	mProjHashClearPass->setProjHashTexture(mProjHashTexture.get());
	mProjHashClearPass->dispatch(texSize.x, texSize.y, 1);
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_ShaderImageAccess | MemorySync_TextureFetch);

	mProjHashPass->bind();
	mProjHashPass->setDepthTexture(depth);
	mProjHashPass->setProjHashTexture(mProjHashTexture.get());
	mProjHashPass->setViewProj(viewProj);
	mProjHashPass->setInvViewProj(invViewProj);
	mProjHashPass->setTexSize(texSize);
	mProjHashPass->setWaterHeight(waterHeight);
	mProjHashPass->setCameraDir(cameraDir);

	mProjHashPass->dispatch(texSize.x, texSize.y, 1);
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

	mProjHashTexture = std::make_unique<Texture2D>(width, height, desc, nullptr);
}