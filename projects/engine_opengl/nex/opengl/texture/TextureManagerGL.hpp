#pragma once
#include <map>
#include <glad/glad.h>
#include <list>
#include <nex/opengl/texture/SamplerGL.hpp>
#include "nex/gui/Drawable.hpp"


class FileSystem;
class TextureGL;
class CubeMapGL;

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

	TextureGL* createTextureGL(std::string localPathFileName, GLuint textureID);

	TextureGL* getImageGL(const std::string& file);

	TextureGL* getDefaultBlackTexture();
	TextureGL* getDefaultNormalTexture();
	TextureGL* getDefaultWhiteTexture();

	TextureGL* getHDRImage(const std::string& file, TextureData data);
	TextureGL* getHDRImage2(const std::string& file, TextureData data);
	TextureGL* getImage(const std::string& file, TextureData data = { true, true, Linear_Mipmap_Linear, Linear, Repeat, RGBA, BITS_8});

	/**
	 * Initializes the texture manager.
	 * @param textureFileSystem Used to resolve texture file paths
	 */
	void init(FileSystem* textureFileSystem);

	void loadImages(const std::string& imageFolder);
	SamplerGL* getDefaultImageSampler();

	/**
	 * Provides access the texture manager singleton.
	 */
	static TextureManagerGL* get();

	void release();

	void releaseTexture(TextureGL * tex);

protected:
	std::list<TextureGL> textures;
	std::list<CubeMapGL> cubeMaps;
	std::map<std::string, TextureGL*> textureLookupTable;
	nex::Logger m_logger;
	SamplerGL* mDefaultImageSampler;
	FileSystem* mFileSystem;

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