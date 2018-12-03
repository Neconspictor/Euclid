#pragma once
#include <map>
#include <glad/glad.h>
#include <list>
#include <nex/opengl/texture/SamplerGL.hpp>
#include "nex/gui/Drawable.hpp"


namespace nex {
	struct GenericImageGL;
	class FileSystem;
	class TextureGL;
	class CubeMapGL;
}



/**
 * A texture manager for an opengl renderer.
 */
class TextureManagerGL
{
public:

	TextureManagerGL();
	TextureManagerGL(TextureManagerGL&&) = default;
	TextureManagerGL& operator=(TextureManagerGL&&) = default;
	
	TextureManagerGL(const TextureManagerGL&) = delete;
	TextureManagerGL& operator=(const TextureManagerGL&) = delete;
	

	virtual ~TextureManagerGL();

	void init();

	CubeMapGL* addCubeMap(CubeMapGL cubemap);

	CubeMapGL* createCubeMap(const std::string& right, const std::string& left,
		const std::string& top, const std::string& bottom,
		const std::string& back, const std::string& front, bool useSRGBOnCreation = false);

	nex::TextureGL* createTextureGL(std::string localPathFileName, GLuint textureID, int width, int height);

	nex::TextureGL* getImageGL(const std::string& file);

	nex::TextureGL* getDefaultBlackTexture();
	nex::TextureGL* getDefaultNormalTexture();
	nex::TextureGL* getDefaultWhiteTexture();

	nex::TextureGL* getHDRImage(const std::string& file, nex::TextureData data);

	nex::TextureGL* getImage(const std::string& file,
		nex::TextureData data = {
			nex::TextureFilter::Linear_Mipmap_Linear,
			nex::TextureFilter::Linear,
			nex::TextureUVTechnique::Repeat,
			nex::ColorSpace::SRGBA,
			nex::PixelDataType::UBYTE,
			nex::InternFormat::SRGBA8,
			true}
	);

	/**
	 * Initializes the texture manager.
	 * @param textureFileSystem Used to resolve texture file paths
	 */
	void init(nex::FileSystem* textureFileSystem);

	void loadImages(const std::string& imageFolder);
	SamplerGL* getDefaultImageSampler();

	/**
	 * Provides access the texture manager singleton.
	 */
	static TextureManagerGL* get();

	void release();

	void releaseTexture(nex::TextureGL * tex);




	void writeHDR(const nex::GenericImageGL& imageData, const char* filePath);

	void readImage(nex::GenericImageGL* imageData, const char* filePath);
	void writeImage(const nex::GenericImageGL& imageData, const char* filePath);

	void readGLITest(const char* filePath);

protected:
	std::list<nex::TextureGL> textures;
	std::list<CubeMapGL> cubeMaps;
	std::map<std::string, nex::TextureGL*> textureLookupTable;
	nex::Logger m_logger;
	SamplerGL* mDefaultImageSampler;
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