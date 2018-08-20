#pragma once
#include <nex/shader/shader.hpp>

class GaussianBlurHorizontalShader : public ShaderConfig
{
public:
	virtual ~GaussianBlurHorizontalShader() {};
	virtual void setTexture(Texture* tex) = 0;
	virtual void setImageHeight(float height) = 0;
	virtual void setImageWidth(float width) = 0;
};

class GaussianBlurVerticalShader : public ShaderConfig
{
public:
	virtual ~GaussianBlurVerticalShader() {};
	virtual void setTexture(Texture* tex) = 0;
	virtual void setImageHeight(float height) = 0;
	virtual void setImageWidth(float width) = 0;
};