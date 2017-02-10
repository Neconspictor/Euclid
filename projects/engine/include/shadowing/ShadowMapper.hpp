#pragma once

class ShadowMap
{
	ShadowMap(int width, int height)
		: width(width),
		  height(height)
	{
	}

	virtual ~ShadowMap(){}

	int getWidth() const
	{
		return width;
	};
	int getHeight() const
	{
		return height;
	}

protected:
	int width;
	int height;
};

class Scene;

class ShadowMapper
{
public:

	ShadowMapper() {}

	virtual ~ShadowMapper() {}

	/**
	 * Renders a shadow map from a given Scene
	 */
	virtual ShadowMap renderShadowMap(Scene* scene) = 0;

	virtual void init() = 0;

	virtual void reset() = 0;

	virtual void updateBuffers() = 0;
};