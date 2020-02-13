#include <nex/effects/Blit.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/renderer/RenderTypes.hpp>
#include <nex/renderer/Drawer.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/renderer/RenderBackend.hpp>

namespace nex {
	class Blit::BlitPass : public Shader
	{
	public:
		BlitPass(bool blitColor0, bool blitColor1, bool blitDepth, bool useStencilTest) : Shader(), mUseStencilTest(useStencilTest)
		{
			std::vector<std::string> defines;

			if (blitColor0)
				defines.push_back("#define BLIT_COLOR0 1");

			if (blitColor1)
				defines.push_back("#define BLIT_COLOR1 1");

			if (blitDepth)
				defines.push_back("#define BLIT_DEPTH 1");

			if (useStencilTest)
				defines.push_back("#define USE_STENCIL_TEST_FOR_BLITTING 1");

			mProgram = ShaderProgram::create("screen_space_vs.glsl", "blit_fs.glsl", 
				nullptr, nullptr, nullptr,
				defines);

			mColorMap0 = mProgram->createTextureUniform("colorMap0", UniformType::TEXTURE2D, 0);
			mColorMap1 = mProgram->createTextureUniform("colorMap1", UniformType::TEXTURE2D, 1);
			mDepthMap = mProgram->createTextureUniform("depthMap", UniformType::TEXTURE2D, 2);
			mStencilMap = mProgram->createTextureUniform("stencilMap", UniformType::TEXTURE2D, 3);
		}

		void setColor0(const Texture* texture) {
			mProgram->setTexture(texture, Sampler::getPoint(), mColorMap0.bindingSlot);
		}

		void setColor1(const Texture* texture) {
			mProgram->setTexture(texture, Sampler::getPoint(), mColorMap1.bindingSlot);
		}

		void setDepth(const Texture* texture) {
			mProgram->setTexture(texture, Sampler::getPoint(), mDepthMap.bindingSlot);
		}

		void setStencil(const Texture* texture) {
			if (!mUseStencilTest) throw_with_trace(std::runtime_error("Blit::BlitPass::setStencil(): stencil mode isn't set!"));
			mProgram->setTexture(texture, Sampler::getPoint(), mStencilMap.bindingSlot);
		}

	private:
		UniformTex mColorMap0;
		UniformTex mColorMap1;
		UniformTex mDepthMap;
		UniformTex mStencilMap;
		bool mUseStencilTest;
	};

	Blit::Blit() : 
		mBlitPass(std::make_unique<BlitPass>(true, false, false, false)),
		mBlitDepthStencilLumaPass(std::make_unique<BlitPass>(true, true, true, true)),
		mRenderTarget(std::make_unique<RenderTarget>(1,1)) //Note: default width, height is not important for rendering! It is just necessary for creating a render target without any attachments
	{
	}

	Blit::~Blit() = default;

	void Blit::blitColor0Color1DepthUseStencilTest(const RenderTarget& source, const Texture* sourceStencil, 
		const RenderTarget& dest,
		const RenderState& state)
	{
		mBlitDepthStencilLumaPass->bind();
		mBlitDepthStencilLumaPass->setColor0(source.getColorAttachmentTexture(0));
		mBlitDepthStencilLumaPass->setColor1(source.getColorAttachmentTexture(1));
		mBlitDepthStencilLumaPass->setDepth(source.getDepthAttachment()->texture.get());
		mBlitDepthStencilLumaPass->setStencil(sourceStencil);
		
		Drawer::drawFullscreenTriangle(state, mBlitDepthStencilLumaPass.get());
	}

	void Blit::blit(const Texture* color, const RenderState& state)
	{
		mBlitPass->bind();
		mBlitPass->setColor0(color);

		Drawer::drawFullscreenTriangle(state, mBlitPass.get());
	}
}