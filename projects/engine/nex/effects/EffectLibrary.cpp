#include <nex/effects/EffectLibrary.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/effects/SkyBoxPass.hpp>
#include <nex/effects/DepthMapPass.hpp>
#include <nex/effects/SpritePass.hpp>
#include <nex/post_processing/PostProcessor.hpp>
#include <nex/post_processing/DownSampler.hpp>
#include <nex/effects/SimpleColorPass.hpp>
#include <nex/material/Material.hpp>
#include <nex/pbr/IrradianceSphereHullDrawPass.hpp>
#include <nex/effects/Blit.hpp>
#include <nex/common/Log.hpp>

nex::EffectLibrary::EffectLibrary(unsigned width, unsigned height) :
	mGaussianBlur(std::make_unique<GaussianBlur>(width, height)),
	mEquirectangualrSkyBox(std::make_unique<EquirectangularSkyBoxPass>()),
	mPanoramaSkyBox(std::make_unique<PanoramaSkyBoxPass>()),
	mSkyBox(std::make_unique<SkyBoxPass>()),
	mDepthMap(std::make_unique<DepthMapPass>()),
	mSprite(std::make_unique<SpritePass>()),
	mDepthSprite(std::make_unique<DepthSpritePass>()),
	mSimpleColorShader(std::make_unique<SimpleColorPass>()),
	mIrradianceSphereHullDrawShader(std::make_unique<IrradianceSphereHullDrawPass>()),
	mDownSampler(std::make_unique<DownSampler>(width, height)),
	mBlit(std::make_unique<Blit>())
{

	mPostProcessor = std::make_unique<PostProcessor>(width, height, mDownSampler.get(), mGaussianBlur.get());
	LOG(Logger("EffectLibrary"), Info) << "created effect library";
}

nex::EffectLibrary::~EffectLibrary() = default;

nex::Blit* nex::EffectLibrary::getBlit()
{
	return mBlit.get();
}

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

nex::DepthMapPass* nex::EffectLibrary::getDepthMapShader()
{
	return mDepthMap.get();
}

nex::SpritePass* nex::EffectLibrary::getSpritePass()
{
	return mSprite.get();
}

nex::DepthSpritePass* nex::EffectLibrary::getDepthSpritePass()
{
	return mDepthSprite.get();
}

nex::SimpleColorPass* nex::EffectLibrary::getSimpleColorShader()
{
	return mSimpleColorShader.get();
}

nex::IrradianceSphereHullDrawPass* nex::EffectLibrary::getIrradianceSphereHullDrawShader()
{
	return mIrradianceSphereHullDrawShader.get();
}

std::unique_ptr<nex::SimpleColorMaterial> nex::EffectLibrary::createSimpleColorMaterial()
{
	return std::make_unique<SimpleColorMaterial>(mSimpleColorShader.get());
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