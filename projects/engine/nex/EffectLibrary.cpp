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


nex::DownSampler::DownSampler() : mDownSampleShader(std::make_unique<DownSampleShader>())
{
}

nex::DownSampler::~DownSampler() = default;

void nex::DownSampler::downsample(Texture2D* src, RenderTarget2D* dest)
{
	auto* renderBackend = RenderBackend::get();
	dest->bind();
	renderBackend->setViewPort(0,0, dest->getWidth(), dest->getHeight());
	dest->clear(Color);

	mDownSampleShader->bind();
	mDownSampleShader->getProgram()->setTexture(mDownSampleShader->getSourceLocation(), src, 0);

	static auto* vertexArray = StaticMeshManager::get()->getNDCFullscreenPlane();

	vertexArray->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);
}

nex::EffectLibrary::EffectLibrary(unsigned width, unsigned height) :
	mGaussianBlur(std::make_unique<GaussianBlur>()),
	mEquirectangualrSkyBox(std::make_unique<EquirectangularSkyBoxShader>()),
	mPanoramaSkyBox(std::make_unique<PanoramaSkyBoxShader>()),
	mSkyBox(std::make_unique<SkyBoxShader>()),
	mDepthMap(std::make_unique<DepthMapShader>()),
	mShadow(std::make_unique<ShadowShader>()),
	mScreen(std::make_unique<ScreenShader>()),
	mPostProcessor(std::make_unique<PostProcessor>(width, height))
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