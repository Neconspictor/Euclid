#pragma once

class Texture
{
public:
	Texture() {}
	virtual ~Texture() {}
};

class RenderTarget
{
public:
	virtual ~RenderTarget() {};
	virtual Texture* getTexture() = 0;
};

class DepthMap
{
public:
	explicit DepthMap(int width, int height)
	{
		this->width = width;
		this->height = height;
	};
	virtual ~DepthMap() {}

	int getWidth() const
	{
		return width;
	}

	int getHeight() const
	{
		return height;
	}

	virtual Texture* getTexture() = 0;

protected:
	int width, height;
};