#pragma once

class Texture
{
public:
	Texture() {}
	virtual ~Texture() {}
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

protected:
	int width, height;
};