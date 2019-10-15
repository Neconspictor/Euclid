#include <nex/effects/Blit.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/renderer/RenderTypes.hpp>

namespace nex {
	class Blit::BlitPass : public Pass
	{
	public:
		BlitPass(bool useStencilTest) : Pass(), mUseStencilTest(useStencilTest)
		{
			std::vector<std::string> defines;

			if (useStencilTest)
				defines.push_back("#define USE_STENCIL_TEST 1");

			mShader = Shader::create("screen_space_vs.glsl", "blit_fs.glsl", 
				nullptr, nullptr, nullptr,
				defines);

			mColorMap = mShader->createTextureUniform("colorMap", UniformType::TEXTURE2D, 0);
			mStencilMap = mShader->createTextureUniform("stencilMap", UniformType::TEXTURE2D, 1);

			mSampler.setMinFilter(TextureFilter::NearestNeighbor);
			mSampler.setMagFilter(TextureFilter::NearestNeighbor);
		}

		void setColor(Texture* texture) {
			mShader->setTexture(texture, &mSampler, mColorMap.bindingSlot);
		}

		void setStencil(Texture* texture) {
			if (!mUseStencilTest) throw_with_trace(std::runtime_error("Blit::BlitPass::setStencil(): stencil mode isn't set!"));
			mShader->setTexture(texture, &mSampler, mStencilMap.bindingSlot);
		}

	private:
		UniformTex mColorMap;
		UniformTex mStencilMap;
		bool mUseStencilTest;
	};

	Blit::Blit() : 
		mBlitPass(std::make_unique<BlitPass>(false)),
		mBlitStencilPass(std::make_unique<BlitPass>(true))
	{
	}

	Blit::~Blit() = default;

	void Blit::blitStencil(Texture * color, Texture * stencil)
	{
		mBlitStencilPass->bind();
		mBlitStencilPass->setColor(color);
		mBlitStencilPass->setStencil(stencil);
		
		const auto& state = RenderState::getNoDepthTest();
	}

	void Blit::blit(Texture* color)
	{
		mBlitPass->bind();
		mBlitPass->setColor(color);
	}

}