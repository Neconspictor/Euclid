#include <nex/EffectLibrary.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <nex/shader/DepthMapShader.hpp>
#include <nex/shader/ShadowShader.hpp>
#include <nex/shader/ScreenShader.hpp>
#include <nex/post_processing/PostProcessor.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/post_processing/DownSampler.hpp>
#include <nex/shader/post_processing/blur/GaussianBlurShader.hpp>


nex::EffectLibrary::EffectLibrary(unsigned width, unsigned height) :
	mGaussianBlur(std::make_unique<GaussianBlur>(width, height)),
	mEquirectangualrSkyBox(std::make_unique<EquirectangularSkyBoxPass>()),
	mPanoramaSkyBox(std::make_unique<PanoramaSkyBoxPass>()),
	mSkyBox(std::make_unique<SkyBoxPass>()),
	mDepthMap(std::make_unique<DepthMapShader>()),
	mShadow(std::make_unique<ShadowPass>()),
	mScreen(std::make_unique<ScreenShader>()),
	mDownSampler(std::make_unique<DownSampler>(width, height))
{
	mPostProcessor = std::make_unique<PostProcessor>(width, height, mDownSampler.get(), mGaussianBlur.get());
}

nex::EffectLibrary::~EffectLibrary() = default;

nex::GaussianBlur* nex::EffectLibrary::getGaussianBlur()
{
	return mGaussianBlur.get();
}

nex::EquirectangularSkyBoxPass* nex::EffectLibrary::getEquirectangularSkyBoxShader()
{
	return mEquirectangualrSkyBox.get();
}

nex::PanoramaSkyBoxPass* nex::EffectLibrary::getPanoramaSkyBoxShader()
{
	return mPanoramaSkyBox.get();
}

nex::SkyBoxPass* nex::EffectLibrary::getSkyBoxShader()
{
	return mSkyBox.get();
}

nex::DepthMapShader* nex::EffectLibrary::getDepthMapShader()
{
	return mDepthMap.get();
}

nex::ShadowPass* nex::EffectLibrary::getShadowVisualizer()
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
	mGaussianBlur->resize(width, height);
}