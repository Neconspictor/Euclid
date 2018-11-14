#pragma once
#include <glad/glad.h>
#include <nex/util/Math.hpp>

class RenderTargetGL;
class RendererOpenGL;
class CubeRenderTargetGL;
class BaseRenderTargetGL;

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
	RGBA,
	RG,
};

enum Resolution {
	BITS_8,
	BITS_16,
	BITS_32,
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
	Resolution resolution;
};


class TextureGL
{
public:
	explicit TextureGL();
	TextureGL(GLuint texture);
	TextureGL(TextureGL&& o);
	TextureGL& operator=(TextureGL&& o);


	TextureGL(const TextureGL&) = delete;
	TextureGL& operator=(const TextureGL&) = delete;

	virtual ~TextureGL();

	GLuint getTexture() const;

	void setTexture(GLuint id);


	static GLint mapFilter(TextureFilter filter);
	static GLint mapUVTechnique(TextureUVTechnique technique);
	static GLuint getFormat(ColorSpace colorspace);
	static GLuint getFormat(int numberComponents);
	static GLuint getInternalFormat(GLuint format, bool useSRGB, bool isFloatData, Resolution resolution);
	static GLuint getType(bool isFloatData);

	static GLuint rgba_float_resolutions[3];
	static GLuint rgb_float_resolutions[3];
	static GLuint rg_float_resolutions[3];

	virtual void release();


protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	friend RenderTargetGL;
	friend CubeRenderTargetGL;
	friend BaseRenderTargetGL;
	GLuint textureID;
};

class RenderBufferGL : public TextureGL {
public:
	RenderBufferGL();
	virtual ~RenderBufferGL();
	RenderBufferGL(GLuint texture);

	RenderBufferGL(RenderBufferGL&& o);
	RenderBufferGL& operator=(RenderBufferGL&& o);

	RenderBufferGL(const RenderBufferGL&) = delete;
	RenderBufferGL& operator=(const RenderBufferGL&) = delete;

	void release() override;
};


class CubeMapGL : public TextureGL
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

	explicit CubeMapGL();
	CubeMapGL(GLuint cubeMap);
	CubeMapGL(CubeMapGL&& o) = default;
	CubeMapGL& operator=(CubeMapGL&& o) = default;

	CubeMapGL(const CubeMapGL& other) = delete;
	CubeMapGL& operator=(const CubeMapGL& other) = delete;

	virtual ~CubeMapGL() = default;

	/**
	 * Maps a cube map side to a corresponding opengl view axis 
	 */
	static GLuint mapCubeSideToSystemAxis(Side side);

	/**
	 *  Generates mipmaps for the current content of this cubemap.
	 */
	void generateMipMaps();

	GLuint getCubeMap() const;

	/**
	 * Provides a 'look at' view matrix for a specific cubemap side
	 * The returned view matrix is for right handed coordinate systems
	 */
	static const glm::mat4& getViewLookAtMatrixRH(Side side);

	void setCubeMap(GLuint id);

	friend CubeRenderTargetGL;

protected:
	static glm::mat4 rightSide;
	static glm::mat4 leftSide;
	static glm::mat4 topSide;
	static glm::mat4 bottomSide;
	static glm::mat4 frontSide;
	static glm::mat4 backSide;
};

class BaseRenderTargetGL {
public:
	explicit BaseRenderTargetGL(int width, int height, GLuint frameBuffer);
	virtual ~BaseRenderTargetGL();

	BaseRenderTargetGL(BaseRenderTargetGL&& o);
	BaseRenderTargetGL& operator=(BaseRenderTargetGL&& o);

	BaseRenderTargetGL(const BaseRenderTargetGL& other) = delete;
	BaseRenderTargetGL& operator=(const BaseRenderTargetGL& other) = delete;

	void copyFrom(BaseRenderTargetGL* dest, const Dimension& sourceDim, int components);

	virtual GLuint getFrameBuffer();
	virtual void setFrameBuffer(GLuint newValue);

	int getHeight() const
	{
		return height;
	}

	int getWidth() const
	{
		return width;
	}

private:
	void swap(BaseRenderTargetGL& o);

protected:
	int width, height;
	GLuint frameBuffer;
};


class CubeRenderTargetGL : public BaseRenderTargetGL
{
public:
	explicit CubeRenderTargetGL(int width, int height, TextureData data);
	
	CubeRenderTargetGL(CubeRenderTargetGL&&) = default;
	CubeRenderTargetGL& operator=(CubeRenderTargetGL&&) = default;


	CubeRenderTargetGL(const CubeRenderTargetGL&) = delete;
	CubeRenderTargetGL& operator=(const CubeRenderTargetGL&) = delete;
	
	virtual ~CubeRenderTargetGL();



	CubeMapGL* createCopy();

	GLuint getRenderBuffer();
	GLuint getCubeMapGL();
	CubeMapGL* getCubeMap();
	GLuint getRendertargetTexture();

	inline int getHeightMipLevel(unsigned int mipMapLevel) const {
		return (int)(height * std::pow(0.5, mipMapLevel));
	}

	inline int getWidthMipLevel(unsigned int mipMapLevel) const {
		return (int)(height * std::pow(0.5, mipMapLevel));
	}

	void release();

	void resizeForMipMap(unsigned int mipMapLevel);

	void setRenderBuffer(GLuint newValue);
	void setCubeMapResult(GLuint newValue);
	void setRenderTargetTexture(GLuint newValue);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	//friend CubeRenderTargetGL;
	GLuint renderBuffer;
	GLuint renderTargetTexture;
	CubeMapGL cubeMapResult;
	TextureData data;
};


class RenderTargetGL : public BaseRenderTargetGL
{
public:
	explicit RenderTargetGL(int width, int height);

	RenderTargetGL(RenderTargetGL&& other) = default;
	RenderTargetGL& operator=(RenderTargetGL&& other) = default;

	RenderTargetGL(const RenderTargetGL& other) = delete;
	RenderTargetGL& operator=(const RenderTargetGL& other) = delete;


	virtual ~RenderTargetGL();

	static RenderTargetGL createMultisampled(int width, int height, const TextureData& data,
		GLuint samples, GLuint depthStencilType);

	static RenderTargetGL createSingleSampled(int width, int height, const TextureData& data, GLuint depthStencilType);

	static RenderTargetGL createVSM(int width, int height);

	GLuint getRenderBuffer();
	GLuint getTextureGL();
	TextureGL* getTexture();

	void release();

	void setRenderBuffer(GLuint newValue);
	void setTextureBuffer(GLuint newValue);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	friend CubeRenderTargetGL;
	TextureGL textureBuffer;
	GLuint renderBuffer;
};


class CubeDepthMapGL : public BaseRenderTargetGL, public TextureGL
{
public:
	explicit CubeDepthMapGL(int width, int height);
	
	CubeDepthMapGL(CubeDepthMapGL&& other) = default;
	CubeDepthMapGL& operator=(CubeDepthMapGL&& other) =default;

	CubeDepthMapGL(const CubeDepthMapGL& other) = delete;
	CubeDepthMapGL& operator=(const CubeDepthMapGL& other) = delete;

	virtual ~CubeDepthMapGL();

	GLuint getCubeMapTexture() const;
	CubeMapGL* getCubeMap();
	GLuint getFramebuffer() const;

private:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	CubeMapGL cubeMap;
	glm::mat4 matrices[6];
};


class DepthMapGL : public BaseRenderTargetGL
{
public:
	explicit DepthMapGL(int width, int height);
	
	DepthMapGL(DepthMapGL&& other) = default;
	DepthMapGL& operator=(DepthMapGL&& other) = default;


	DepthMapGL(const DepthMapGL& other) = delete;
	DepthMapGL& operator=(const DepthMapGL& other) = delete;
	

	virtual ~DepthMapGL();

	GLuint getFramebuffer() const;
	GLuint getTexture() const;
	TextureGL* getTexture();

	void release();

private:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	TextureGL texture;
};

class PBR_GBufferGL : public BaseRenderTargetGL {
public:
	explicit PBR_GBufferGL(int width, int height);
	PBR_GBufferGL(PBR_GBufferGL&& o) = default;
	PBR_GBufferGL& operator=(PBR_GBufferGL&& o) = default;


	PBR_GBufferGL(const PBR_GBufferGL&) = delete;
	PBR_GBufferGL& operator=(const PBR_GBufferGL&) = delete;

	virtual ~PBR_GBufferGL() {}

	TextureGL* getAlbedo();
	TextureGL* getAoMetalRoughness();
	TextureGL* getNormal();
	TextureGL* getPosition();
	TextureGL* getDepth();


protected:
	TextureGL albedo;
	TextureGL aoMetalRoughness;
	TextureGL normal;
	TextureGL position;
	RenderBufferGL depth;
};

class OneTextureRenderTarget : public BaseRenderTargetGL {
public:
	OneTextureRenderTarget(GLuint frameBuffer,
		TextureGL texture,
		unsigned int width,
		unsigned int height);

	virtual ~OneTextureRenderTarget();

	TextureGL* getTexture();

private:
	TextureGL m_texture;
};