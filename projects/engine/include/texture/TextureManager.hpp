#pragma once
#include <string>
#include <texture/Texture.hpp>

enum TextureFilter
{
	NearestNeighbor, 
	Linear,
	Bilinear, 
	Near_Mipmap_Near,     // trilinear filtering with double nearest neighbor filtering
	Near_Mipmap_Linear,   // trilinear filtering from nearest neighbor to bilinear filtering
	Linear_Mipmap_Near,   // trilinear filtering from bilinear to nearest neighbor filtering
	Linear_Mipmap_Linear, // trilinear filtering from bilinear to bilinear filtering
};

enum TextureUVTechnique
{
	ClampToEdge,
	Repeat,
};

enum ColorSpace {
	RGB,
	RGBA
};


struct TextureData
{
	bool useSRGB;
	bool generateMipMaps;
	TextureFilter minFilter;  // minification filter
	TextureFilter magFilter;  // magnification filter
	TextureUVTechnique uvTechnique;
	ColorSpace colorspace;
	bool isFloatData; //specifies whether the data should be interpreted as float data
};

class TextureManager
{
public:

	TextureManager() {}

	virtual ~TextureManager(){}

	virtual CubeMap* createCubeMap(const std::string& right, const std::string& left, 
		const std::string& top, const std::string& bottom, 
		const std::string& back, const std::string& front, bool useSRGBOnCreation = false) = 0;

	virtual Texture* getDefaultBlackTexture() = 0;
	virtual Texture* getDefaultNormalTexture() = 0;
	virtual Texture* getDefaultWhiteTexture() = 0;

	virtual Texture* getHDRImage(const std::string& file, TextureData data = { true, true, Linear_Mipmap_Linear, Linear, Repeat, RGBA, false}) = 0;
	virtual Texture* getImage(const std::string& file, TextureData data = {true, true, Linear_Mipmap_Linear, Linear, Repeat, RGBA, false }) = 0;


	virtual std::string getImagePath() = 0;

	/**
	 * Loads all images from a specified image folder and all its sub folders.
	 */
	virtual void loadImages(const std::string& imageFolder) = 0;

	virtual void releaseTexture(Texture* tex) = 0;
};