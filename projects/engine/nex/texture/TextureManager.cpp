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
#include <nex/exception/ResourceLoadException.hpp>

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

	void TextureManager::addToCache(std::unique_ptr<Texture2D> texture, const std::filesystem::path& path)
	{
		auto* ptr = texture.get();
		textures.emplace_back(std::move(texture));
		textureLookupTable.insert(std::pair<std::filesystem::path, nex::Texture2D*>(path, ptr));
	}

	void TextureManager::init(const std::filesystem::path& textureRootPath, 
		const std::filesystem::path& compiledTextureRootPath, 
		const std::string& compiledTextureFileExtension,
		const std::string& metaFileExtension,
		const std::string& embeddedTextureFileExtension)
	{
		std::vector<std::filesystem::path> includeDirectories = {textureRootPath};
		mFileSystem = std::make_unique<FileSystem>(includeDirectories, compiledTextureRootPath, compiledTextureFileExtension);
		mTextureRootDirectory = textureRootPath;
		mMetaFileExt = metaFileExtension;
		mEmbeddedTextureFileExt = embeddedTextureFileExtension;
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
			true,
			{
				TexFilter::Linear_Mipmap_Linear,
				TexFilter::Linear,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				InternalFormat::RGB8,
				true
			});
	}

	Texture2D * TextureManager::getDefaultNormalTexture()
	{
		//normal maps shouldn't use mipmaps (important for shading!)
		return getImage("_intern/default_normal.png",
			true,
			{
				TexFilter::Linear_Mipmap_Linear,
				TexFilter::Linear,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				InternalFormat::RGB8,
				true
			});
	}

	Texture2D * TextureManager::getDefaultWhiteTexture()
	{
		return getImage("_intern/white.png",
			true,
			{
				TexFilter::Linear_Mipmap_Linear,
				TexFilter::Linear,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				UVTechnique::Repeat,
				InternalFormat::RGB8,
				true
			});
	}

	const std::string& TextureManager::getEmbeddedTextureFileExtension() const
	{
		return mEmbeddedTextureFileExt;
	}

	nex::FileSystem * TextureManager::getFileSystem()
	{
		return mFileSystem.get();
	}

	Texture2D* TextureManager::getImage(const std::filesystem::path& file, bool flipY, const TextureDesc& data, bool detectColorSpace)
	{
		const auto resolvedPath = mFileSystem->resolvePath(file);

		auto it = textureLookupTable.find(resolvedPath);

		// Don't create duplicate textures!
		if (it != textureLookupTable.end())
		{
			return it->second;
		}

		LOG(m_logger, Debug) << "texture to load: " << resolvedPath;

		auto image = loadImage(file, flipY, data, detectColorSpace);
		textures.emplace_back(std::move(image));


		auto* result = textures.back().get();

		textureLookupTable.insert(std::pair<std::filesystem::path, nex::Texture2D*>(resolvedPath, result));

		return result;
	}

	std::unique_ptr<nex::Texture2D> TextureManager::loadImageUnsafe(const std::filesystem::path& file, bool flipY, const nex::TextureDesc& data, bool detectColorSpace)
	{
		StoreImage storeImage;

		std::filesystem::path compiledResource = mFileSystem->getCompiledPath(file).path;

		if (std::filesystem::exists(compiledResource))
		{
			FileSystem::load(compiledResource, storeImage);

		}
		else
		{
			storeImage.mipmapCount = 1;
			storeImage.images.resize(1);
			storeImage.images[0].resize(1);
			storeImage.textureTarget = TextureTarget::TEXTURE2D;
			storeImage.tileCount = glm::uvec2(1);

			auto& genericImage = storeImage.images[0][0];

			const auto resolvedPath = mFileSystem->resolvePath(file);

			auto extension = file.extension().generic_string();
			std::transform(extension.begin(), extension.end(), extension.begin(), std::tolower);

			// Note we load each texture with 32 bit float precision, but the internal texture format decides which format the texture uses 
			if (extension == ".hdr")
				genericImage = ImageFactory::loadFloat(resolvedPath, isSRGB(data.internalFormat), flipY, detectColorSpace ? 0 : getComponents(data.internalFormat));
			else
				genericImage = ImageFactory::loadUByte(resolvedPath, isSRGB(data.internalFormat), flipY, detectColorSpace ? 0 : getComponents(data.internalFormat));


			loadTextureMeta(resolvedPath, storeImage);

			FileSystem::store(compiledResource, storeImage);
		}

		return createTexture(storeImage, data, detectColorSpace);
	}

	std::unique_ptr<nex::Texture2D> TextureManager::createTexture(const StoreImage& storeImage, const nex::TextureDesc& data, bool detectColorSpace)
	{
		std::unique_ptr<nex::Texture2D> texture;

		const auto& image = storeImage.images[0][0];

		TextureTransferDesc transfer;
		transfer.imageDesc = image.desc;
		transfer.data = (void*)image.pixels.getPixels();
		transfer.dataByteSize = image.pixels.getBufferSize();
		transfer.mipMapLevel = 0;
		transfer.xOffset = transfer.yOffset = transfer.zOffset = 0;

		if (detectColorSpace)
		{
			const auto colorspace = image.desc.colorspace;
			const auto pixelDataType = image.desc.pixelDataType;
			const auto channels = getComponents(colorspace);
			TextureDesc copy = data;
			copy.internalFormat = isLinear(copy.internalFormat) ? getInternalFormat(channels, copy.internalFormat) :
				getGammaInternalFormat(channels);

			texture = std::make_unique<Texture2D>(image.desc.width, image.desc.height, copy, &transfer);
		}
		else
		{
			texture = std::make_unique<Texture2D>(image.desc.width, image.desc.height, data, &transfer);
		}

		texture->setTileCount(storeImage.tileCount);

		return texture;
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

	InternalFormat TextureManager::getInternalFormat(unsigned channels, InternalFormat baseFormat)
	{
		const auto type = nex::getType(baseFormat);
		const bool isFloat = type == InternalFormatType::FLOAT;
		if (!(isFloat || type == InternalFormatType::NORMAL)) {
			throw_with_trace(std::runtime_error("Not supported internal format for auto channel recognition: " + std::to_string((int)baseFormat)));
		}

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

	bool TextureManager::isLinear(InternalFormat internFormat)
	{
		return internFormat != InternalFormat::SRGB8 && internFormat != InternalFormat::SRGBA8;
	}

	void TextureManager::loadTextureMeta(const std::filesystem::path& absoluteTexturePath, StoreImage& storeImage)
	{
		std::filesystem::path metaFile = absoluteTexturePath;
		metaFile += mMetaFileExt;

		Configuration config;
		unsigned defaultVal = 1;
		
		config.addOption("Texture", "tileCountX", &storeImage.tileCount.x, defaultVal);
		config.addOption("Texture", "tileCountY", &storeImage.tileCount.y, defaultVal);

		if (!config.load(metaFile)) {
			storeImage.tileCount = glm::uvec2(defaultVal);
		}
	}

	std::unique_ptr<nex::Texture2D> TextureManager::loadImage(const std::filesystem::path& file, bool flipY, const nex::TextureDesc& data, bool detectColorSpace)
	{
		std::unique_ptr<nex::Texture2D> texture;
		try {
			texture = loadImageUnsafe(file, flipY, data, detectColorSpace);
		}
		catch (std::exception & e) {
			throw_with_trace(e);
		}
		catch (...) {
			throw_with_trace(nex::ResourceLoadException("Unknown error occurred while loading texture " + file.generic_string()));
		}
		
		return texture;
	}

	std::unique_ptr<nex::Texture2D> TextureManager::loadEmbeddedImage(const std::filesystem::path& file, const unsigned char* data, 
		int dataSize, bool flipY, const nex::TextureDesc& desc, bool detectColorSpace)
	{

		std::unique_ptr<nex::Texture2D> texture;

		try {
			StoreImage storeImage;
			std::filesystem::path compiledResource = mFileSystem->getCompiledPath(file).path;
			storeImage.mipmapCount = 1;
			storeImage.images.resize(1);
			storeImage.images[0].resize(1);
			storeImage.textureTarget = TextureTarget::TEXTURE2D;
			storeImage.tileCount = glm::uvec2(1);

			auto& genericImage = storeImage.images[0][0];
			genericImage = ImageFactory::loadUByte(data, dataSize, isSRGB(desc.internalFormat), flipY, detectColorSpace ? 0 : getComponents(desc.internalFormat));

			FileSystem::store(compiledResource, storeImage);
			texture = createTexture(storeImage, desc, detectColorSpace);
		}
		catch (std::exception & e) {
			throw_with_trace(e);
		}
		catch (...) {
			throw_with_trace(nex::ResourceLoadException("Unknown error occurred while loading texture " + file.generic_string()));
		}

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