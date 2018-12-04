#pragma once
#include <map>
#include <list>
#include <nex/texture/Sampler.hpp>
#include <nex/gui/Drawable.hpp>


namespace nex {
	struct GenericImage;
	class FileSystem;

	/**
 * A texture manager for an opengl renderer.
 */
	class TextureManagerGL
	{
	public:

		TextureManagerGL();

		TextureManagerGL(const TextureManagerGL&) = delete;
		TextureManagerGL& operator=(const TextureManagerGL&) = delete;


		virtual ~TextureManagerGL();

		void init();

		void addCubeMap(CubeMap* cubemap);

		CubeMap* createCubeMap(const std::string& right, const std::string& left,
			const std::string& top, const std::string& bottom,
			const std::string& back, const std::string& front, bool useSRGBOnCreation = false);

		nex::Texture* createTextureGL(std::string localPathFileName, GLuint textureID, int width, int height);

		nex::Texture* getDefaultBlackTexture();
		nex::Texture* getDefaultNormalTexture();
		nex::Texture* getDefaultWhiteTexture();

		nex::Texture* getHDRImage(const std::string& file, const nex::TextureData& data);

		nex::Texture* getImage(const std::string& file,
			const nex::TextureData& data = {
				nex::TextureFilter::Linear_Mipmap_Linear,
				nex::TextureFilter::Linear,
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
		 */
		static TextureManagerGL* get();

		void release();

		void releaseTexture(nex::Texture * tex);




		void writeHDR(const nex::GenericImage& imageData, const char* filePath);

		void readImage(nex::GenericImage* imageData, const char* filePath);
		void writeImage(const nex::GenericImage& imageData, const char* filePath);

		void readGLITest(const char* filePath);

	protected:
		std::list<nex::Texture*> textures;
		std::list<CubeMap*> cubeMaps;
		std::map<std::string, nex::Texture*> textureLookupTable;
		nex::Logger m_logger;
		Sampler* mDefaultImageSampler;
		nex::FileSystem* mFileSystem;

	private:

		static TextureManagerGL instance;
	};

	class TextureManager_Configuration : public nex::engine::gui::Drawable
	{
	public:
		TextureManager_Configuration(TextureManagerGL* textureManager);

	protected:
		void drawSelf() override;

		TextureManagerGL* m_textureManager;
	};
}