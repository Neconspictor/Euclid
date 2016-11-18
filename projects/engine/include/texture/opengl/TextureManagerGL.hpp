#ifndef ENGINE_TEXTURE_OPENGL_TEXTURE_MANAGER_GL_HPP
#define ENGINE_TEXTURE_OPENGL_TEXTURE_MANAGER_GL_HPP
#include <texture/TextureManager.hpp>
#include <map>
#include <GL/glew.h>
#include <memory>

/**
 * A texture manager for an opengl renderer.
 */
class TextureManagerGL : public TextureManager
{
public:

	virtual ~TextureManagerGL() override;

	virtual GLuint getImage(const std::string& file);

	virtual void loadImages(const std::string& imageFolder) override;

	/**
	 * Provides access the texture manager singleton.
	 */
	static TextureManagerGL* get();

protected:
	std::map<std::string, GLuint> textures;

private:
	// this class is a singleton, thus private constructor
	TextureManagerGL(); 

	static std::unique_ptr<TextureManagerGL> instance;
};
#endif
