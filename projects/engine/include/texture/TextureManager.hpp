#pragma once
#include <string>
#include <texture/Texture.hpp>
#include "CubeMap.hpp"

enum TextureFilter
{
	NearestNeighbor, 
	Bilinear, 
	Near_Near,     // trilinear filtering with double nearest neighbor filtering
	Near_Linear,   // trilinear filtering from nearest neighbor to bilinear filtering
	Linear_Near,   // trilinear filtering from bilinear to nearest neighbor filtering
	Linear_Linear, // trilinear filtering from bilinear to bilinear filtering
};

enum TextureUVTechnique
{
	ClampToEdge,
	Repeat,
};


struct TextureData
{
	bool useSRGB;
	bool generateMipMaps;
	TextureFilter minFilter;  // minification filter
	TextureFilter magFilter;  // magnification filter
	TextureUVTechnique uvTechnique;
};

class TextureManager
{
public:

	TextureManager() {}

	virtual ~TextureManager(){}

	virtual CubeMap* createCubeMap(const std::string& right, const std::string& left, 
		const std::string& top, const std::string& bottom, 
		const std::string& back, const std::string& front, bool useSRGBOnCreation = false) = 0;

	virtual Texture* getHDRImage(const std::string& file, TextureData data = { true, true, Linear_Linear, Bilinear, Repeat }) = 0;
	virtual Texture* getImage(const std::string& file, TextureData data = {true, true, Linear_Linear, Bilinear, Repeat}) = 0;


	virtual std::string getImagePath() = 0;

	/**
	 * Loads all images from a specified image folder and all its sub folders.
	 */
	virtual void loadImages(const std::string& imageFolder) = 0;
};