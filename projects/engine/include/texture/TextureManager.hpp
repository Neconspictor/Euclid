#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP
#include <string>
#include <texture/Texture.hpp>

class TextureManager
{
public:

	TextureManager() {}

	virtual ~TextureManager(){}

	virtual Texture* getImage(const std::string& file) = 0;

	/**
	 * Loads all images from a specified image folder and all its sub folders.
	 */
	virtual void loadImages(const std::string& imageFolder) = 0;
};

#endif