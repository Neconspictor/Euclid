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

		/**
		 * Initializes the texture manager.
		 * @param textureRootPath Used to resolve texture file paths
		 */
		void init(std::filesystem::path textureRootPath, std::filesystem::path compiledTextureRootPath, std::string compiledTextureFileExtension);

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

		nex::Texture2D* getImage(const std::string& file,
			const nex::TextureData& data = {
				nex::TextureFilter::Linear_Mipmap_Linear,
				nex::TextureFilter::Linear,
				nex::TextureUVTechnique::Repeat,
				nex::TextureUVTechnique::Repeat,
				nex::TextureUVTechnique::Repeat,
				nex::ColorSpace::SRGBA,
				nex::PixelDataType::UBYTE,
				nex::InternFormat::SRGBA8,
				true }, bool detectColorSpace = false
		);

		std::unique_ptr<nex::Texture2D> loadImage(const std::string& file,
			const nex::TextureData& data = {
				nex::TextureFilter::Linear_Mipmap_Linear,
				nex::TextureFilter::Linear,
				nex::TextureUVTechnique::Repeat,
				nex::TextureUVTechnique::Repeat,
				nex::TextureUVTechnique::Repeat,
				nex::ColorSpace::SRGBA,
				nex::PixelDataType::UBYTE,
				nex::InternFormat::SRGBA8,
				true }, bool detectColorSpace = false
		);

		Sampler* getDefaultImageSampler();
		Sampler* getPointSampler();

		/**
		 * Provides access the texture manager singleton.
		 * NOTE: Has to be initialized on first use
		 */
		static TextureManager* get();

		void release();

		void releaseTexture(nex::Texture * tex);


		//void readGLITest(const char* filePath);

	protected:

		static ColorSpace getColorSpace(unsigned channels);
		static ColorSpace getGammaSpace(unsigned channels);

		InternFormat getInternalFormat(unsigned channels);
		InternFormat getGammaInternalFormat(unsigned channels);

		static bool isLinear(ColorSpace colorspace);
		static bool isLinear(InternFormat internFormat);

		std::list<std::unique_ptr<Texture2D>> textures;
		std::list<CubeMap> cubeMaps;
		std::map<std::string, Texture2D*> textureLookupTable;
		nex::Logger m_logger;
		std::unique_ptr<Sampler> mDefaultImageSampler;
		std::unique_ptr<Sampler> mPointSampler;
		std::unique_ptr<nex::FileSystem> mFileSystem;
		std::filesystem::path mTextureRootDirectory;
		std::filesystem::path mCompiledTextureRootDirectory;
		std::filesystem::path mCompiledTextureFileExtension;
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
