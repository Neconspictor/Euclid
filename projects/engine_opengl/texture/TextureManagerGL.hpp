#pragma once
#include <texture/TextureManager.hpp>
#include <map>
#include <glad/glad.h>
#include <memory>
#include <platform/logging/LoggingClient.hpp>
#include <list>


class TextureGL;
class CubeMapGL;

/**
 * A texture manager for an opengl renderer.
 */
class TextureManagerGL : public TextureManager
{
public:

	TextureManagerGL();
	TextureManagerGL(TextureManagerGL&&) = default;
	TextureManagerGL& operator=(TextureManagerGL&&) = default;
	
	TextureManagerGL(const TextureManagerGL&) = delete;
	TextureManagerGL& operator=(const TextureManagerGL&) = delete;
	

	virtual ~TextureManagerGL() override;

	CubeMapGL* addCubeMap(CubeMapGL cubemap);

	CubeMap* createCubeMap(const std::string& right, const std::string& left,
		const std::string& top, const std::string& bottom,
		const std::string& back, const std::string& front, bool useSRGBOnCreation = false) override;

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


private:

	static TextureManagerGL instance;
};