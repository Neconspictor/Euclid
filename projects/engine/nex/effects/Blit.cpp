#include <nex/effects/Blit.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/renderer/RenderTypes.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>

namespace nex {
	class Blit::BlitPass : public Pass
	{
	public:
		BlitPass(bool useStencilTest, bool useDepth, bool useLuminance) : Pass(), mUseStencilTest(useStencilTest)
		{
			std::vector<std::string> defines;

			if (useStencilTest)
				defines.push_back("#define USE_STENCIL_TEST 1");

			if (useDepth)
				defines.push_back("#define USE_DEPTH 1");

			if (useLuminance)
				defines.push_back("#define USE_LUMINANCE 1");

			mShader = Shader::create("screen_space_vs.glsl", "blit_fs.glsl", 
				nullptr, nullptr, nullptr,
				defines);

			mColorMap = mShader->createTextureUniform("colorMap", UniformType::TEXTURE2D, 0);
			mLuminanceMap = mShader->createTextureUniform("luminanceMap", UniformType::TEXTURE2D, 1);
			mDepthMap = mShader->createTextureUniform("depthMap", UniformType::TEXTURE2D, 2);
			mStencilMap = mShader->createTextureUniform("stencilMap", UniformType::TEXTURE2D, 3);
		}

		void setColor(Texture* texture) {
			mShader->setTexture(texture, Sampler::getPoint(), mColorMap.bindingSlot);
		}

		void setLuminance(Texture* texture) {
			mShader->setTexture(texture, Sampler::getPoint(), mLuminanceMap.bindingSlot);
		}

		void setDepth(Texture* texture) {
			mShader->setTexture(texture, Sampler::getPoint(), mDepthMap.bindingSlot);
		}

		void setStencil(Texture* texture) {
			if (!mUseStencilTest) throw_with_trace(std::runtime_error("Blit::BlitPass::setStencil(): stencil mode isn't set!"));
			mShader->setTexture(texture, Sampler::getPoint(), mStencilMap.bindingSlot);
		}

	private:
		UniformTex mColorMap;
		UniformTex mLuminanceMap;
		UniformTex mDepthMap;
		UniformTex mStencilMap;
		bool mUseStencilTest;
	};

	Blit::Blit() : 
		mBlitPass(std::make_unique<BlitPass>(false, false, false)),
		mBlitDepthStencilLumaPass(std::make_unique<BlitPass>(true, true, true))
	{
	}

	Blit::~Blit() = default;

	void Blit::blitDepthStencilLuma(Texture * color, Texture* luminance, Texture* depth, Texture * stencil, const RenderState& state)
	{
		mBlitDepthStencilLumaPass->bind();
		mBlitDepthStencilLumaPass->setColor(color);
		mBlitDepthStencilLumaPass->setLuminance(luminance);
		mBlitDepthStencilLumaPass->setDepth(depth);
		mBlitDepthStencilLumaPass->setStencil(stencil);
		
		StaticMeshDrawer::drawFullscreenTriangle(state, mBlitDepthStencilLumaPass.get());
	}

	void Blit::blit(Texture* color, const RenderState& state)
	{
		mBlitPass->bind();
		mBlitPass->setColor(color);

		StaticMeshDrawer::drawFullscreenTriangle(state, mBlitPass.get());
	}
}