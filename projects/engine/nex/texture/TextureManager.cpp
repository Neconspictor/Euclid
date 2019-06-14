#include <nex/util/ExceptionHandling.hpp>
#include <imgui/imgui.h>

/*#include <gli/gli.hpp>
#include <gli/texture2d.hpp>
#include <gli/load.hpp>
#include <gli/save.hpp>*/

#include <nex/FileSystem.hpp>
#include <nex/texture/Image.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/texture/TextureSamplerData.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/texture/Texture.hpp>

using namespace std;

namespace nex {


	TextureManager::TextureManager() : m_logger("TextureManagerGL"), mDefaultImageSampler(nullptr), mFileSystem(nullptr)
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

	void TextureManager::init(std::filesystem::path textureRootPath, std::filesystem::path compiledTextureRootPath, std::string compiledTextureFileExtension)
	{
		std::vector<std::filesystem::path> includeDirectories = {textureRootPath};
		mFileSystem = std::make_unique<FileSystem>(std::move(includeDirectories));
		mCompiledTextureRootDirectory = std::move(compiledTextureRootPath);
		mCompiledTextureFileExtension = std::move(compiledTextureFileExtension);
		mTextureRootDirectory = textureRootPath;

		mDefaultImageSampler = std::make_unique<Sampler>(SamplerDesc());
		mDefaultImageSampler->setMinFilter(TextureFilter::Linear_Mipmap_Linear);
		mDefaultImageSampler->setMagFilter(TextureFilter::Linear);
		mDefaultImageSampler->setWrapR(TextureUVTechnique::Repeat);
		mDefaultImageSampler->setWrapS(TextureUVTechnique::Repeat);
		mDefaultImageSampler->setWrapT(TextureUVTechnique::Repeat);
		mDefaultImageSampler->setAnisotropy(16.0f);

		mPointSampler = std::make_unique<Sampler>(SamplerDesc());
		mPointSampler->setMinFilter(TextureFilter::NearestNeighbor);
		mPointSampler->setMagFilter(TextureFilter::NearestNeighbor);
		mPointSampler->setWrapR(TextureUVTechnique::ClampToEdge);
		mPointSampler->setWrapS(TextureUVTechnique::ClampToEdge);
		mPointSampler->setWrapT(TextureUVTechnique::ClampToEdge);
		mPointSampler->setAnisotropy(1.0f);
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
				TextureFilter::Linear_Mipmap_Linear,
				TextureFilter::Linear,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				ColorSpace::RGB,
				PixelDataType::UBYTE,
				InternFormat::RGB8,
				true
			});
	}

	Texture2D * TextureManager::getDefaultNormalTexture()
	{
		//normal maps shouldn't use mipmaps (important for shading!)
		return getImage("_intern/default_normal.png",
			{
				TextureFilter::Linear_Mipmap_Linear,
				TextureFilter::Linear,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				ColorSpace::RGB,
				PixelDataType::UBYTE,
				InternFormat::RGB8,
				true
			});
	}

	Texture2D * TextureManager::getDefaultWhiteTexture()
	{
		return getImage("_intern/white.png",
			{
				TextureFilter::Linear_Mipmap_Linear,
				TextureFilter::Linear,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				ColorSpace::RGB,
				PixelDataType::UBYTE,
				InternFormat::RGB8,
				true
			});
	}

	Texture2D* TextureManager::getImage(const string& file, const TextureData& data)
	{
		const auto resolvedPath = mFileSystem->resolvePath(file).generic_string();

		auto it = textureLookupTable.find(resolvedPath);

		// Don't create duplicate textures!
		if (it != textureLookupTable.end())
		{
			return it->second;
		}

		LOG(m_logger, Debug) << "texture to load: " << resolvedPath;

		auto image = loadImage(file, true, data);
		textures.emplace_back(std::move(image));


		auto* result = textures.back().get();

		textureLookupTable.insert(std::pair<std::string, nex::Texture2D*>(resolvedPath, result));

		return result;
	}

	std::unique_ptr<nex::Texture2D> TextureManager::loadImage(const std::string& file, bool flip, const nex::TextureData& data)
	{
		GenericImage image;
		std::filesystem::path resource = file;

		if (resource.is_absolute())
		{
			if (!FileSystem::isContained(resource, mTextureRootDirectory)) throw_with_trace(std::invalid_argument("file isn't contained in texture root directory!"));
			resource = std::filesystem::relative(resource, mTextureRootDirectory);
		}

		if (resource == std::filesystem::path("nature\\tileable_wood_texture.jpg"))
		{
			bool test = false;
		}

		std::filesystem::path compiledResource = mCompiledTextureRootDirectory / resource.replace_extension(mCompiledTextureFileExtension);

		if (std::filesystem::exists(compiledResource))
		{
			FileSystem::load(compiledResource, image);

		} else
		{
			const auto resolvedPath = mFileSystem->resolvePath(file).generic_string();
			if (data.pixelDataType == PixelDataType::FLOAT)
			{
				image = ImageFactory::loadHDR(resolvedPath.c_str(), flip, getComponents(data.colorspace));
			}
			else
			{
				image = ImageFactory::loadNonHDR(resolvedPath.c_str(), flip, getComponents(data.colorspace));
			}

			FileSystem::store(compiledResource, image);
		}


		auto texture = std::make_unique<Texture2D>(image.width, image.height, data, image.pixels.getPixels());

		return texture;
	}

	Sampler* TextureManager::getDefaultImageSampler()
	{
		return mDefaultImageSampler.get();
	}
	Sampler* TextureManager::getPointSampler()
	{
		return mPointSampler.get();
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
		mDefaultImageSampler.reset(nullptr);
		mPointSampler.reset(nullptr);
	}


	TextureManager_Configuration::TextureManager_Configuration(TextureManager* textureManager) : m_textureManager(textureManager)
	{
	}

	void TextureManager_Configuration::drawSelf()
	{

		Sampler* sampler = m_textureManager->getDefaultImageSampler();
		float anisotropy = sampler->getState().maxAnisotropy;

		//float anisotropyBackup = anisotropy;

		// render configuration properties
		ImGui::PushID(m_id.c_str());
		if (ImGui::InputFloat("Anisotropic Filtering (read-only)", &anisotropy, 1.0f, 1.0f, "%.3f")) //ImGuiInputTextFlags_ReadOnly
		{
			sampler->setAnisotropy(anisotropy);
		}
		ImGui::PopID();

		//throw_with_trace(std::runtime_error("Hello exception!"));
	}
}