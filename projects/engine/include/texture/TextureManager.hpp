#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP
#include <string>

class TextureManager
{
public:

	TextureManager() {}

	virtual ~TextureManager(){}

	/**
	 * Loads all images from a specified image folder and all its sub folders.
	 */
	virtual void loadImages(const std::string& imageFolder) = 0;
};

#endif