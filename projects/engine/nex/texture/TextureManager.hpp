#pragma once
#include <map>
#include <list>
#include <nex/gui/Drawable.hpp>
#include "nex/common/Log.hpp"
#include <nex/texture/TextureSamplerData.hpp>


namespace nex {
	class CubeMap;
	struct GenericImage;
	class FileSystem;
	class Sampler;
	class Texture;
	class Texture2D;
	struct StoreImage;

	/**
 * A texture manager for an opengl renderer.
 */
	class TextureManager
	{
	public:

		TextureManager();

		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;


		virtual ~TextureManager();

		void addToCache(std::unique_ptr<Texture2D> texture, const std::filesystem::path& path);

		/**
		 * Initializes the texture manager.
		 * @param textureRootPath Used to resolve texture file paths
		 */
		void init(const std::filesystem::path& resourceRootPath, 
			const std::filesystem::path& compiledResourceRootPath, 
			const std::string& compiledTextureFileExtension,
			const std::string& metaFileExtension,
			const std::string& embeddedTextureFileExtension);

		/**
		 * Flips the y axis of an image
		 * Note: imageSize has to have at least width * height bytes!
		 * @param pitch : The byte size(!) of one row
		 * @param height : The amount of rows
		 */
		static void flipYAxis(char* imageSource, size_t pitch, size_t height);

		nex::Texture2D* getDefaultBlackTexture();
		nex::Texture2D* getDefaultNormalTexture();
		nex::Texture2D* getDefaultWhiteTexture();

		const std::string& getEmbeddedTextureFileExtension() const;

		nex::FileSystem* getFileSystem();

		nex::Texture2D* getImage(const std::filesystem::path& file,
			bool flipY = true,
			const nex::TextureDesc& data = {
				nex::TexFilter::Linear_Mipmap_Linear,
				nex::TexFilter::Linear,
				nex::UVTechnique::Repeat,
				nex::UVTechnique::Repeat,
				nex::UVTechnique::Repeat,
				nex::InternalFormat::SRGBA8,
				true }, bool detectColorSpace = false
		);

		std::unique_ptr<nex::Texture2D> loadImage(const std::filesystem::path& file,
			bool flipY = true,
			const nex::TextureDesc& data = {
				nex::TexFilter::Linear_Mipmap_Linear,
				nex::TexFilter::Linear,
				nex::UVTechnique::Repeat,
				nex::UVTechnique::Repeat,
				nex::UVTechnique::Repeat,
				nex::InternalFormat::SRGBA8,
				true }, bool detectColorSpace = false
		);

		std::unique_ptr<nex::Texture2D> loadEmbeddedImage(const std::filesystem::path& file,
			const unsigned char* data,
			int dataSize,
			bool flipY = true,
			const nex::TextureDesc& desc = {
				nex::TexFilter::Linear_Mipmap_Linear,
				nex::TexFilter::Linear,
				nex::UVTechnique::Repeat,
				nex::UVTechnique::Repeat,
				nex::UVTechnique::Repeat,
				nex::InternalFormat::SRGBA8,
				true }, bool detectColorSpace = false
				);


		/**
		 * Provides access the texture manager singleton.
		 * NOTE: Has to be initialized on first use
		 */
		static TextureManager* get();

		void release();

		void releaseTexture(nex::Texture * tex);


		//void readGLITest(const char* filePath);

	protected:

		std::unique_ptr<nex::Texture2D> loadImageUnsafe(
			const std::filesystem::path& file,
			bool flipY,
			const nex::TextureDesc& data, bool detectColorSpace
		);

		std::unique_ptr<nex::Texture2D> createTexture(const StoreImage& storeImage, const nex::TextureDesc& data, bool detectColorSpace);


		static ColorSpace getColorSpace(unsigned channels);

		InternalFormat getInternalFormat(unsigned channels, InternalFormat baseFormat);
		InternalFormat getGammaInternalFormat(unsigned channels);

		//static bool isLinear(ColorSpace colorspace);
		static bool isLinear(InternalFormat internFormat);

		void loadTextureMeta(const std::filesystem::path& absoluteTexturePath, StoreImage& storeImage);

		std::list<std::unique_ptr<Texture2D>> textures;
		std::list<CubeMap> cubeMaps;
		std::map<std::filesystem::path, Texture2D*> textureLookupTable;
		nex::Logger m_logger;
		std::unique_ptr<nex::FileSystem> mFileSystem;
		std::filesystem::path mResourceRootDirectory;
		std::string mMetaFileExt;
		std::string mEmbeddedTextureFileExt;
	};

	class TextureManager_Configuration : public nex::gui::Drawable
	{
	public:
		TextureManager_Configuration(TextureManager* textureManager);

	protected:
		void drawSelf() override;

		TextureManager* m_textureManager;
	};
}
