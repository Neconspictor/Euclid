#pragma once
#include <map>
#include <list>
#include <nex/texture/Sampler.hpp>
#include <nex/gui/Drawable.hpp>
#include <nex/texture/Texture.hpp>
#include "nex/common/Log.hpp"


namespace nex {
	struct GenericImage;
	class FileSystem;

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

		void init();

		void addCubeMap(CubeMap* cubemap);

		CubeMap* createCubeMap(const std::string& right, const std::string& left,
			const std::string& top, const std::string& bottom,
			const std::string& back, const std::string& front, bool useSRGBOnCreation = false);

		nex::Texture* getDefaultBlackTexture();
		nex::Texture* getDefaultNormalTexture();
		nex::Texture* getDefaultWhiteTexture();

		nex::Texture* getHDRImage(const std::string& file, const nex::TextureData& data);

		nex::Texture* getImage(const std::string& file,
			const nex::TextureData& data = {
				nex::TextureFilter::Linear_Mipmap_Linear,
				nex::TextureFilter::Linear,
				nex::TextureUVTechnique::Repeat,
				nex::TextureUVTechnique::Repeat,
				nex::TextureUVTechnique::Repeat,
				nex::ColorSpace::SRGBA,
				nex::PixelDataType::UBYTE,
				nex::InternFormat::SRGBA8,
				true }
		);

		/**
		 * Initializes the texture manager.
		 * @param textureFileSystem Used to resolve texture file paths
		 */
		void init(nex::FileSystem* textureFileSystem);

		void loadImages(const std::string& imageFolder);
		Sampler* getDefaultImageSampler();

		/**
		 * Provides access the texture manager singleton.
		 * NOTE: Has to be initialized on first use
		 */
		static TextureManager* get();

		void release();

		void releaseTexture(nex::Texture * tex);




		void writeHDR(const nex::GenericImage& imageData, const char* filePath);

		void readImage(nex::GenericImage* imageData, const char* filePath);
		void writeImage(const nex::GenericImage& imageData, const char* filePath);

		//void readGLITest(const char* filePath);

	protected:
		std::list<nex::Texture*> textures;
		std::list<CubeMap*> cubeMaps;
		std::map<std::string, nex::Texture*> textureLookupTable;
		nex::Logger m_logger;
		Sampler* mDefaultImageSampler;
		nex::FileSystem* mFileSystem;
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
