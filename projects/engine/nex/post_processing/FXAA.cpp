#include <nex/post_processing/FXAA.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/material/Material.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/renderer/Drawer.hpp>


class nex::FXAA::FxaaPass : public Shader 
{
public:
	FxaaPass(bool useGamma) : Shader()
	{
		std::vector<std::string> defines;
		if (useGamma) {
			defines.push_back("#define SOURCE_GAMMA_SPACE");
		}

		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/fxaa_fs.glsl", 
			nullptr, nullptr, nullptr, defines);

		mInverseFrameBufferSize = { mProgram->getUniformLocation("inverseFramebufferSize"), UniformType::VEC2};
		mSourceTexture = mProgram->createTextureUniform("sourceTexture", UniformType::TEXTURE2D, 0);
	}

	void setInverseFrameBufferSize(const glm::vec2& size) {
		mProgram->setVec2(mInverseFrameBufferSize.location, size);
	}

	void setSource(Texture* texture) {
		mProgram->setTexture(texture, Sampler::getLinear(), mSourceTexture.bindingSlot);
	}

	const RenderState& getState() const {
		return RenderState::getNoDepthTest();
	}

private:

	Uniform mInverseFrameBufferSize;
	UniformTex mSourceTexture;
};

nex::FXAA::FXAA() : 
mFxaaPassGamma(std::make_unique<FxaaPass>(true)),
mFxaaPassLinear(std::make_unique<FxaaPass>(false))
{
}

nex::FXAA::~FXAA() = default;

void nex::FXAA::antialias(Texture* source, bool sourceIsInGammaSpace)
{
	FxaaPass* pass = nullptr;
	if (sourceIsInGammaSpace) pass = mFxaaPassGamma.get();
	else pass = mFxaaPassLinear.get();

	pass->bind();
	glm::vec2 inverseSize(1.0f / float(source->getWidth()), 1.0f / float(source->getHeight()));
	pass->setInverseFrameBufferSize(inverseSize);
	pass->setSource(source);
	Drawer::drawFullscreenTriangle(pass->getState(), pass);
}