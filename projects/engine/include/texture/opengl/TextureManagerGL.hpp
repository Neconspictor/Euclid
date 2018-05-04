#pragma once
#include <texture/TextureManager.hpp>
#include <map>
#include <glad/glad.h>
#include <memory>
#include <platform/logging/LoggingClient.hpp>
#include <texture/opengl/TextureGL.hpp>

/**
 * A texture manager for an opengl renderer.
 */
class TextureManagerGL : public TextureManager
{
public:
	virtual ~TextureManagerGL() override;

	CubeMapGL* addCubeMap(CubeMapGL cubemap);

	CubeMap* createCubeMap(const std::string& right, const std::string& left,
		const std::string& top, const std::string& bottom,
		const std::string& back, const std::string& front, bool useSRGBOnCreation = false) override;

	CubeMap* createCubeMap(int sideWidth, int sideHeight, TextureData data) override;

	TextureGL* createTextureGL(std::string localPathFileName, GLuint textureID);

	TextureGL* getImageGL(const std::string& file);

	virtual Texture* getDefaultBlackTexture() override;
	virtual Texture* getDefaultNormalTexture() override;
	virtual Texture* getDefaultWhiteTexture() override;

	virtual Texture* getHDRImage(const std::string& file, TextureData data) override;
	virtual Texture* getHDRImage2(const std::string& file, TextureData data);
	virtual Texture* getImage(const std::string& file, TextureData data = { true, true, Linear_Mipmap_Linear, Linear, Repeat, RGBA, BITS_8}) override;

	std::string getImagePath() override;

	std::string getFullFilePath(const std::string& localFilePath);

	virtual void loadImages(const std::string& imageFolder) override;

	/**
	 * Provides access the texture manager singleton.
	 */
	static TextureManagerGL* get();

	// Inherited via TextureManager
	virtual void releaseTexture(Texture * tex) override;

protected:
	std::list<TextureGL> textures;
	std::list<CubeMapGL> cubeMaps;
	std::map<std::string, TextureGL*> textureLookupTable;
	platform::LoggingClient logClient;

	static GLint mapFilter(TextureFilter filter, bool useMipMaps);
	static GLint mapUVTechnique(TextureUVTechnique technique);
	static GLuint getFormat(ColorSpace colorspace);
	static GLuint getFormat(int numberComponents);
	static GLuint getInternalFormat(GLuint format, bool useSRGB, bool isFloatData, Resolution resolution);
	static GLuint getType(bool isFloatData);

	static GLuint rgba_float_resolutions[3];
	static GLuint rgb_float_resolutions[3];
	static GLuint rg_float_resolutions[3];

private:
	// this class is a singleton, thus private constructor
	TextureManagerGL(); 

	static std::unique_ptr<TextureManagerGL> instance;
};