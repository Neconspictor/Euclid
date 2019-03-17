#include <nex/post_processing/SMAA.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/texture/Sampler.hpp>
#include <extern/SMAA/AreaTex.h>
#include <extern/SMAA/SearchTex.h>

nex::SMAA::SMAA(unsigned width, unsigned height)
{
	resize(width, height);

	// TODO load area and search textures
	TextureData areaDesc;
	areaDesc.wrapR = areaDesc.wrapS = areaDesc.wrapT = TextureUVTechnique::ClampToEdge;
	areaDesc.minFilter = areaDesc.magFilter = TextureFilter::Linear;
	areaDesc.colorspace = ColorSpace::RG;
	areaDesc.internalFormat = InternFormat::RG8;
	areaDesc.pixelDataType = PixelDataType::UBYTE;
	mAreaTex = std::make_unique<Texture2D>(AREATEX_WIDTH, AREATEX_HEIGHT, areaDesc, areaTexBytes);

	TextureData searchDesc;
	searchDesc.wrapR = areaDesc.wrapS = areaDesc.wrapT = TextureUVTechnique::ClampToEdge;
	searchDesc.minFilter = areaDesc.magFilter = TextureFilter::Linear;
	searchDesc.colorspace = ColorSpace::R;
	searchDesc.internalFormat = InternFormat::R8;
	areaDesc.pixelDataType = PixelDataType::UBYTE;
	mSearchTex = std::make_unique<Texture2D>(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, searchDesc, searchTexBytes);

	SamplerDesc samplerDesc;
	samplerDesc.minFilter = samplerDesc.magFilter = TextureFilter::NearestNeighbor;
	samplerDesc.maxAnisotropy = 0.0f;
	samplerDesc.wrapR = areaDesc.wrapS = areaDesc.wrapT = TextureUVTechnique::ClampToEdge;
	mPointFilter = std::make_unique<Sampler>(samplerDesc);

	samplerDesc.minFilter = samplerDesc.magFilter = TextureFilter::Linear;
	mBilinearFilter = std::make_unique<Sampler>(samplerDesc);
}

nex::SMAA::~SMAA() = default;

void nex::SMAA::resize(unsigned width, unsigned height)
{
	TextureData data;
	data.generateMipMaps = false;
	data.minFilter = data.magFilter = TextureFilter::Linear;
	data.colorspace = ColorSpace::RG;
	data.internalFormat = InternFormat::RG16F;
	data.pixelDataType = PixelDataType::FLOAT;
	data.wrapR = data.wrapS  = data.wrapT = TextureUVTechnique::ClampToEdge;

	mEdgesTex = std::make_unique<RenderTarget2D>(width, height, data);
	mBlendTex = std::make_unique<RenderTarget2D>(width, height, data);
}


void nex::SMAA::reset()
{
	mEdgesTex->bind();
	mEdgesTex->clear(RenderComponent::Color); // no depth-/stencil buffer needed

	mBlendTex->bind();
	mBlendTex->clear(RenderComponent::Color); // no depth-/stencil buffer needed
}