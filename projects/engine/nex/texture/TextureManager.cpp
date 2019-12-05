#include <nex/util/ExceptionHandling.hpp>
#include <imgui/imgui.h>

/*#include <gli/gli.hpp>
#include <gli/texture2d.hpp>
#include <gli/load.hpp>
#include <gli/save.hpp>*/

#include <nex/resource/FileSystem.hpp>
#include <nex/texture/Image.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/texture/TextureSamplerData.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/config/Configuration.hpp>

namespace nex {


	TextureManager::TextureManager() : m_logger("TextureManagerGL")
	{
	}

	void TextureManager::releaseTexture(Texture * tex)
	{
		for (auto&& it = textures.begin(); it != textures.end(); ++it) {
			if ((it->get()) == tex) {
				textures.erase(it);
				break; // we're done
			}
		}
	}


	TextureManager::~TextureManager()
	{
		release();
	}

	void TextureManager::init(const std::filesystem::path& textureRootPath, 
		const std::filesystem::path& compiledTextureRootPath, 
		const std::string& compiledTextureFileExtension,
		const std::string& metaFileExtension)
	{
		std::vector<std::filesystem::path> includeDirectories = {textureRootPath};
		mFileSystem = std::make_unique<FileSystem>(includeDirectories, compiledTextureRootPath, compiledTextureFileExtension);
		mTextureRootDirectory = textureRootPath;
		mMetaFileExt = metaFileExtension;
	}

	void TextureManager::flipYAxis(char* imageSource, size_t pitch, size_t height)
	{
		std::vector<char> backupRowVec(pitch);
		auto* backupRow = backupRowVec.data();

		for (size_t i = 0; i < height; ++i)
		{
			// backup current row
			auto* currentRow = imageSource + i * pitch;
			memcpy_s(backupRow, pitch, currentRow, pitch);

			// get the y-axis mirrored row
			auto mirroredRowIndex = height - i - 1;
			auto* mirroredRow = imageSource + mirroredRowIndex * pitch;

			// Replace the current row with the mirrored row
			memcpy_s(currentRow, pitch, mirroredRow, pitch);

			// Replace the mirrored row with the backup
			memcpy_s(mirroredRow, pitch, backupRow, pitch);
		}
	}

	Texture2D * TextureManager::getDefaultBlackTexture()
	{
		return getImage("_intern/black.png",
			{
				TexFilter::Linear_Mipmap_Linear,
				TexFilter::Linear,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				ColorSpace::RGB,
				PixelDataType::UBYTE,
				InternalFormat::RGB8,
				true
			});
	}

	Texture2D * TextureManager::getDefaultNormalTexture()
	{
		//normal maps shouldn't use mipmaps (important for shading!)
		return getImage("_intern/default_normal.png",
			{
				TexFilter::Linear_Mipmap_Linear,
				TexFilter::Linear,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				ColorSpace::RGB,
				PixelDataType::UBYTE,
				InternalFormat::RGB8,
				true
			});
	}

	Texture2D * TextureManager::getDefaultWhiteTexture()
	{
		return getImage("_intern/white.png",
			{
				TexFilter::Linear_Mipmap_Linear,
				TexFilter::Linear,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				ColorSpace::RGB,
				PixelDataType::UBYTE,
				InternalFormat::RGB8,
				true
			});
	}

	nex::FileSystem * TextureManager::getFileSystem()
	{
		return mFileSystem.get();
	}

	Texture2D* TextureManager::getImage(const std::filesystem::path& file, const TextureDesc& data, bool detectColorSpace)
	{
		const auto resolvedPath = mFileSystem->resolvePath(file);

		auto it = textureLookupTable.find(resolvedPath.u8string());

		// Don't create duplicate textures!
		if (it != textureLookupTable.end())
		{
			return it->second;
		}

		LOG(m_logger, Debug) << "texture to load: " << resolvedPath;

		auto image = loadImage(file, data, detectColorSpace);
		textures.emplace_back(std::move(image));


		auto* result = textures.back().get();

		textureLookupTable.insert(std::pair<std::string, nex::Texture2D*>(resolvedPath.u8string(), result));

		return result;
	}

	ColorSpace TextureManager::getColorSpace(unsigned channels)
	{
		switch(channels)
		{
		case 1:
			return ColorSpace::R;
		case 2:
			return ColorSpace::RG;
		case 3:
			return ColorSpace::RGB;
		case 4:
			return ColorSpace::RGBA;
		default:
			throw std::runtime_error("Not supported channel number: " + std::to_string(channels));
		}

		return ColorSpace::R;
	}

	ColorSpace TextureManager::getGammaSpace(unsigned channels)
	{
		switch (channels)
		{
		case 3:
			return ColorSpace::SRGB;
		case 4:
			return ColorSpace::SRGBA;
		default:
			throw std::runtime_error("Not supported channel number: " + std::to_string(channels));
		}

		return ColorSpace::SRGB;
	}

	InternalFormat TextureManager::getInternalFormat(unsigned channels, bool isFloat)
	{
		switch (channels)
		{
		case 1:
			if (isFloat) return InternalFormat::R32F;
			return InternalFormat::R8;
		case 2:
			if (isFloat) return InternalFormat::RG32F;
			return InternalFormat::RG8;
		case 3:
			if (isFloat) return InternalFormat::RGB32F;
			return InternalFormat::RGB8;
		case 4:
			if (isFloat) return InternalFormat::RGBA32F;
			return InternalFormat::RGBA8;
		default:
			throw std::runtime_error("Not supported channel number: " + std::to_string(channels));
		}

		return InternalFormat::R8;
	}

	InternalFormat TextureManager::getGammaInternalFormat(unsigned channels)
	{
		switch (channels)
		{
		case 3:
			return InternalFormat::SRGB8;
		case 4:
			return InternalFormat::SRGBA8;
		default:
			throw std::runtime_error("Not supported channel number: " + std::to_string(channels));
		}

		return InternalFormat::SRGB8;
	}

	bool TextureManager::isLinear(ColorSpace colorspace)
	{
		return colorspace != ColorSpace::SRGB && colorspace != ColorSpace::SRGBA;
	}

	bool TextureManager::isLinear(InternalFormat internFormat)
	{
		return internFormat != InternalFormat::SRGB8 && internFormat != InternalFormat::SRGBA8;
	}

	void TextureManager::loadTextureMeta(const std::string& absoluteTexturePath, StoreImage& storeImage)
	{
		auto metaFile = absoluteTexturePath + mMetaFileExt;

		Configuration config;
		unsigned defaultVal = 1;
		
		config.addOption("Texture", "tileCountX", &storeImage.tileCount.x, defaultVal);
		config.addOption("Texture", "tileCountY", &storeImage.tileCount.y, defaultVal);

		if (!config.load(metaFile)) {
			storeImage.tileCount = glm::uvec2(defaultVal);
		}
	}

	std::unique_ptr<nex::Texture2D> TextureManager::loadImage(const std::filesystem::path& file, const nex::TextureDesc& data, bool detectColorSpace)
	{
		StoreImage storeImage;

		std::filesystem::path compiledResource = mFileSystem->getCompiledPath(file).path;

		if (std::filesystem::exists(compiledResource))
		{
			FileSystem::load(compiledResource, storeImage);

		} else
		{
			storeImage.mipmapCount = 1;
			storeImage.images.resize(1);
			storeImage.images[0].resize(1);
			storeImage.textureTarget = TextureTarget::TEXTURE2D;
			storeImage.tileCount = glm::uvec2(1);

			const auto resolvedPath = mFileSystem->resolvePath(file).generic_u8string();
			if (data.pixelDataType == PixelDataType::FLOAT)
			{
				storeImage.images[0][0] = ImageFactory::loadHDR(resolvedPath.c_str(), detectColorSpace ? 0 : getComponents(data.colorspace));
			}
			else
			{
				storeImage.images[0][0] = ImageFactory::loadNonHDR(resolvedPath.c_str(), detectColorSpace ? 0 : getComponents(data.colorspace));
			}

			loadTextureMeta(resolvedPath, storeImage);

			FileSystem::store(compiledResource, storeImage);
		}

		std::unique_ptr<nex::Texture2D> texture;

		const auto& image = storeImage.images[0][0];

		if (detectColorSpace)
		{
			TextureDesc copy = data;
			copy.colorspace = isLinear(copy.colorspace) ?  getColorSpace(image.channels) : getGammaSpace(image.channels);
			copy.internalFormat = isLinear(copy.internalFormat) ? getInternalFormat(image.channels, copy.pixelDataType == PixelDataType::FLOAT) : 
				getGammaInternalFormat(image.channels);

			texture = std::make_unique<Texture2D>(image.width, image.height, copy, image.pixels.getPixels());
		} else
		{
			texture = std::make_unique<Texture2D>(image.width, image.height, data, image.pixels.getPixels());
		}

		texture->setTileCount(storeImage.tileCount);

		return texture;
	}

	TextureManager* TextureManager::get()
	{
		static TextureManager instance;
		return &instance;
	}

	void TextureManager::release()
	{
		textures.clear();
		cubeMaps.clear();

		textureLookupTable.clear();
	}


	TextureManager_Configuration::TextureManager_Configuration(TextureManager* textureManager) : m_textureManager(textureManager)
	{
	}

	void TextureManager_Configuration::drawSelf()
	{

		Sampler* sampler = Sampler::getDefaultImage();
		float anisotropy = sampler->getState().maxAnisotropy;

		//float anisotropyBackup = anisotropy;

		// render configuration properties
		ImGui::PushID(mId.c_str());
		if (ImGui::InputFloat("Anisotropic Filtering (read-only)", &anisotropy, 1.0f, 1.0f, "%.3f")) //ImGuiInputTextFlags_ReadOnly
		{
			sampler->setAnisotropy(anisotropy);
		}
		ImGui::PopID();

		//throw_with_trace(std::runtime_error("Hello exception!"));
	}
}