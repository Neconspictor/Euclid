#pragma once
#include <string>
#include <nex/texture/Texture.hpp>
#include "Sampler.hpp"
#include "nex/gui/Drawable.hpp"

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

	virtual Texture* getHDRImage(const std::string& file, TextureData data = { true, true, Linear_Mipmap_Linear, Linear, Repeat, RGBA, false, BITS_8}) = 0;
	virtual Texture* getImage(const std::string& file, TextureData data = {true, true, Linear_Mipmap_Linear, Linear, Repeat, RGBA, false, BITS_8 }) = 0;


	virtual std::string getImagePath() = 0;

	/**
	 * Loads all images from a specified image folder and all its sub folders.
	 */
	virtual void loadImages(const std::string& imageFolder) = 0;

	virtual void releaseTexture(Texture* tex) = 0;


	virtual void setAnisotropicFiltering(float value) = 0;

	virtual float getAnisotropicFiltering() const = 0;

	virtual float getMaxAnisotropicFiltering() const = 0;
};

class TextureManager_Configuration : public nex::engine::gui::Drawable
{
public:
	TextureManager_Configuration(TextureManager* textureManager);

protected:
	void drawSelf() override;

	TextureManager* m_textureManager;
};