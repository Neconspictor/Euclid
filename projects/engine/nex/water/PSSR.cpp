#include <nex/water/PSSR.hpp>
#include <nex/texture/Texture.hpp>

nex::PSSR::~PSSR() = default;

nex::Texture2D* nex::PSSR::getProjHashBuffer()
{
	return mProjHasBuffer.get();
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

	mProjHasBuffer = std::make_unique<Texture2D>(width, height, desc, nullptr);

}
