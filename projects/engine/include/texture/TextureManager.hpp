#pragma once
#include <string>
#include <texture/Texture.hpp>
#include "CubeMap.hpp"

class TextureManager
{
public:

	TextureManager() {}

	virtual ~TextureManager(){}

	virtual CubeMap* createCubeMap(const std::string& right, const std::string& left, 
		const std::string& top, const std::string& bottom, 
		const std::string& back, const std::string& front) = 0;

	virtual Texture* getImage(const std::string& file) = 0;

	/**
	 * Loads all images from a specified image folder and all its sub folders.
	 */
	virtual void loadImages(const std::string& imageFolder) = 0;
};