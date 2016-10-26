#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP
#include <GL/glew.h>
#include <string>
#include <map>

class TextureManager
{
public:
	static TextureManager* getInstance();
	static void release();

	GLuint loadImage(const std::string& file);

protected:
	TextureManager();
	bool init();
	virtual ~TextureManager();
	static TextureManager* instance;
	std::map<std::string, GLuint> textures;
};

#endif