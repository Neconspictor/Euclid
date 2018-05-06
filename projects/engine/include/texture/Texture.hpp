#pragma once

#include <glm/glm.hpp>

class Texture
{
public:
	Texture() {}
	virtual ~Texture() {}
};

class CubeMap
{
public:

	/**
	 * Specifies the sides of a cubemap in relation to a coordinate system.
	 * E.g. in a right handed coordinate system POSITIVE_X would specifiy the right side.
	 */
	enum Side {
		POSITIVE_X,
		NEGATIVE_X,
		POSITIVE_Y,
		NEGATIVE_Y,
		POSITIVE_Z,
		NEGATIVE_Z
	};

	/**
	 * Provides a 'look at' view matrix for a specific cubemap side
	 * The returned view matrix is for right handed coordinate systems
	 */
	static const glm::mat4& getViewLookAtMatrixRH(Side side);

	CubeMap() {}
	virtual ~CubeMap() {}

	/**
	 *  Generates mipmaps for the current content of this cubemap.
	 */
	virtual void generateMipMaps() = 0;

protected:
	static glm::mat4 rightSide;
	static glm::mat4 leftSide;
	static glm::mat4 topSide;
	static glm::mat4 bottomSide;
	static glm::mat4 frontSide;
	static glm::mat4 backSide;
};

class CubeRenderTarget
{
public:

	explicit CubeRenderTarget(int width, int height)
	{
		this->width = width;
		this->height = height;
	};

	virtual ~CubeRenderTarget() {};

	inline int getHeight() const
	{
		return height;
	}

	virtual CubeMap* getCubeMap() = 0;

	inline int getHeightMipLevel(unsigned int mipMapLevel) const {
		return height * std::pow(0.5, mipMapLevel);
	}

	virtual CubeMap* createCopy() = 0;

	virtual void resizeForMipMap(unsigned int mipMapLevel) = 0;

	inline int getWidth() const
	{
		return width;
	}

	inline int getWidthMipLevel(unsigned int mipMapLevel) const {
		return height * std::pow(0.5, mipMapLevel);
	}

protected:
	int width, height;
};

class RenderTarget
{
public:

	explicit RenderTarget(int width, int height)
	{
		this->width = width;
		this->height = height;
	};

	virtual ~RenderTarget() {};
	
	int getHeight() const
	{
		return height;
	}
	
	virtual Texture* getTexture() = 0;

	int getWidth() const
	{
		return width;
	}

protected:
	int width, height;
};

class CubeDepthMap
{
public:
	explicit CubeDepthMap(int width, int height)
	{
		this->width = width;
		this->height = height;
	};
	virtual ~CubeDepthMap() {}

	int getWidth() const
	{
		return width;
	}

	int getHeight() const
	{
		return height;
	}

	virtual CubeMap* getCubeMap() = 0;

protected:
	int width, height;
	glm::mat4 matrices[6];
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