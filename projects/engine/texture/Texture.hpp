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

class BaseRenderTarget
{
public:
	explicit BaseRenderTarget(int width, int height)
	{
		this->width = width;
		this->height = height;
	};

	virtual ~BaseRenderTarget() {};

	virtual int getHeight() const
	{
		return height;
	}

	virtual int getWidth() const
	{
		return width;
	}

protected:
	int width, height;
};

class CubeRenderTarget : public virtual BaseRenderTarget
{
public:

	explicit CubeRenderTarget(int width, int height) : BaseRenderTarget(width, height){};

	virtual ~CubeRenderTarget() {};

	virtual CubeMap* getCubeMap() = 0;

	inline int getHeightMipLevel(unsigned int mipMapLevel) const {
		return (int)(height * std::pow(0.5, mipMapLevel));
	}

	virtual CubeMap* createCopy() = 0;

	virtual void resizeForMipMap(unsigned int mipMapLevel) = 0;

	inline int getWidthMipLevel(unsigned int mipMapLevel) const {
		return (int)(height * std::pow(0.5, mipMapLevel));
	}
};

class RenderTarget : public virtual BaseRenderTarget
{
public:

	explicit RenderTarget(int width, int height) : BaseRenderTarget(width, height) {};

	virtual ~RenderTarget() {};

	virtual Texture* getTexture() = 0;
};

class CubeDepthMap : public virtual BaseRenderTarget
{
public:
	explicit CubeDepthMap(int width, int height) : BaseRenderTarget(width, height) {};
	virtual ~CubeDepthMap() {}

	virtual CubeMap* getCubeMap() = 0;

protected:
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

class PBR_GBuffer : public virtual BaseRenderTarget  {
public:
	explicit PBR_GBuffer(int width, int height) : BaseRenderTarget(width, height) {}

	virtual Texture* getAlbedo() = 0;
	virtual Texture* getAO() = 0;
	virtual Texture* getNormal() = 0;
	virtual Texture* getMetal() = 0;
	virtual Texture* getPosition() = 0;
	virtual Texture* getRoughness() = 0;
};