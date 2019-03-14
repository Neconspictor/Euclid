#include <nex/EffectLibrary.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <nex/shader/DepthMapShader.hpp>
#include <nex/shader/ShadowShader.hpp>
#include <nex/shader/ScreenShader.hpp>
#include <nex/post_processing/PostProcessor.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/texture/Sampler.hpp>


class nex::DownSampler::DownSampleShader : public Shader
{
public:
	DownSampleShader()
	{
		mProgram = ShaderProgram::create("fullscreenPlane_vs.glsl", "post_processing/downsample_fs.glsl");
		mSourceUniform = { mProgram->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D };
	}

	const UniformLocation& getSourceLocation() const
	{
		return mSourceUniform.location;
	}

private:

	Uniform mSourceUniform;
};


nex::DownSampler::DownSampler(unsigned width, unsigned height) : mDownSampleShader(std::make_unique<DownSampleShader>()),
mSampler(std::make_unique<Sampler>(SamplerDesc()))
{
	mSampler->setAnisotropy(0.0f);
	mSampler->setMinFilter(TextureFilter::Linear);
	mSampler->setMagFilter(TextureFilter::Linear);
	resize(width, height);
}

nex::DownSampler::~DownSampler() = default;

nex::Texture2D* nex::DownSampler::downsampleHalfResolution(Texture2D* src)
{
	return downsample(src, mHalfResolution.get());
}

nex::Texture2D* nex::DownSampler::downsampleQuarterResolution(Texture2D* src)
{
	return downsample(downsampleHalfResolution(src), mQuarterResolution.get());
}

nex::Texture2D* nex::DownSampler::downsampleEigthResolution(Texture2D* src)
{
	return downsample(downsampleQuarterResolution(src), mEigthResolution.get());
	//return downsample(src, mEigthResolution.get());
}

nex::Texture2D* nex::DownSampler::downsample(Texture2D* src, RenderTarget2D* dest)
{
	auto* renderBackend = RenderBackend::get();
	dest->bind();
	renderBackend->setViewPort(0,0, dest->getWidth(), dest->getHeight());
	dest->clear(Color);

	mDownSampleShader->bind();
	mSampler->bind(0);
	mDownSampleShader->getProgram()->setTexture(mDownSampleShader->getSourceLocation(), src, 0);

	static auto* vertexArray = StaticMeshManager::get()->getNDCFullscreenPlane();

	vertexArray->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);

	return static_cast<Texture2D*>(dest->getColorAttachmentTexture(0));
}

void nex::DownSampler::resize(unsigned width, unsigned height)
{
	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mHalfResolution = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR(), 1);

	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mQuarterResolution = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR(), 1);

	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mEigthResolution = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR(), 1);
}

nex::EffectLibrary::EffectLibrary(unsigned width, unsigned height) :
	mGaussianBlur(std::make_unique<GaussianBlur>()),
	mEquirectangualrSkyBox(std::make_unique<EquirectangularSkyBoxShader>()),
	mPanoramaSkyBox(std::make_unique<PanoramaSkyBoxShader>()),
	mSkyBox(std::make_unique<SkyBoxShader>()),
	mDepthMap(std::make_unique<DepthMapShader>()),
	mShadow(std::make_unique<ShadowShader>()),
	mScreen(std::make_unique<ScreenShader>()),
	mPostProcessor(std::make_unique<PostProcessor>(width, height)),
	mDownSampler(std::make_unique<DownSampler>(width, height))
{

}

nex::EffectLibrary::~EffectLibrary() = default;

nex::GaussianBlur* nex::EffectLibrary::getGaussianBlur()
{
	return mGaussianBlur.get();
}

nex::EquirectangularSkyBoxShader* nex::EffectLibrary::getEquirectangularSkyBoxShader()
{
	return mEquirectangualrSkyBox.get();
}

nex::PanoramaSkyBoxShader* nex::EffectLibrary::getPanoramaSkyBoxShader()
{
	return mPanoramaSkyBox.get();
}

nex::SkyBoxShader* nex::EffectLibrary::getSkyBoxShader()
{
	return mSkyBox.get();
}

nex::DepthMapShader* nex::EffectLibrary::getDepthMapShader()
{
	return mDepthMap.get();
}

nex::ShadowShader* nex::EffectLibrary::getShadowVisualizer()
{
	return mShadow.get();
}

nex::ScreenShader* nex::EffectLibrary::getScreenShader()
{
	return mScreen.get();
}

nex::PostProcessor* nex::EffectLibrary::getPostProcessor()
{
	return mPostProcessor.get();
}

nex::DownSampler* nex::EffectLibrary::getDownSampler()
{
	return mDownSampler.get();
}

void nex::EffectLibrary::resize(unsigned width, unsigned height)
{
	mDownSampler->resize(width, height);
	mPostProcessor->resize(width, height);
}