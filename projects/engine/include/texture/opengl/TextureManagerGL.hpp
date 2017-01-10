#pragma once
#include <texture/TextureManager.hpp>
#include <map>
#include <glad/glad.h>
#include <memory>
#include <platform/logging/LoggingClient.hpp>
#include <texture/opengl/TextureGL.hpp>
#include <texture/opengl/CubeMapGL.hpp>

/**
 * A texture manager for an opengl renderer.
 */
class TextureManagerGL : public TextureManager
{
public:
	virtual ~TextureManagerGL() override;

	CubeMap* createCubeMap(const std::string& right, const std::string& left,
		const std::string& top, const std::string& bottom,
		const std::string& back, const std::string& front) override;

	TextureGL* createTextureGL(std::string localPathFileName, GLuint textureID);

	TextureGL* getImageGL(const std::string& file);

	virtual Texture* getImage(const std::string& file) override;

	std::string getImagePath() override;

	std::string getFullFilePath(const std::string& localFilePath);

	virtual void loadImages(const std::string& imageFolder) override;

	/**
	 * Provides access the texture manager singleton.
	 */
	static TextureManagerGL* get();

protected:
	std::list<TextureGL> textures;
	std::list<CubeMapGL> cubeMaps;
	std::map<std::string, TextureGL*> textureLookupTable;
	platform::LoggingClient logClient;
private:
	// this class is a singleton, thus private constructor
	TextureManagerGL(); 

	static std::unique_ptr<TextureManagerGL> instance;
};