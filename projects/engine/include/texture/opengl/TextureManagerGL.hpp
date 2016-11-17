#ifndef ENGINE_TEXTURE_OPENGL_TEXTURE_MANAGER_GL_HPP
#define ENGINE_TEXTURE_OPENGL_TEXTURE_MANAGER_GL_HPP
#include <texture/TextureManager.hpp>
#include <map>
#include <GL/glew.h>

class TextureManagerGL : public TextureManager
{
public:
	TextureManagerGL();
	virtual ~TextureManagerGL() override;

	GLuint getImage(const std::string& file);

	void loadImages(const std::string& imageFolder) override;

protected:
	std::map<std::string, GLuint> textures;
};
#endif
