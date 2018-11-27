#pragma once
#include <glad/glad.h>
#include <nex/util/Math.hpp>
#include <nex/util/Memory.hpp>

struct StoreImageGL;
class RenderTargetGL;
class RendererOpenGL;
class CubeRenderTargetGL;
class BaseRenderTargetGL;
class CubeMapGL;

enum class TextureFilter
{
	NearestNeighbor = GL_NEAREST,
	Linear = GL_LINEAR,
	Bilinear = GL_LINEAR,
	Near_Mipmap_Near = GL_NEAREST_MIPMAP_NEAREST,     // trilinear filtering with double nearest neighbor filtering
	Near_Mipmap_Linear = GL_NEAREST_MIPMAP_LINEAR,   // trilinear filtering from nearest neighbor to bilinear filtering
	Linear_Mipmap_Near = GL_LINEAR_MIPMAP_NEAREST,   // trilinear filtering from bilinear to nearest neighbor filtering
	Linear_Mipmap_Linear = GL_LINEAR_MIPMAP_LINEAR, // trilinear filtering from bilinear to bilinear filtering
};

enum class TextureUVTechnique
{
	ClampToBorder = GL_CLAMP_TO_BORDER,
	ClampToEdge = GL_CLAMP_TO_EDGE,
	MirrorRepeat = GL_MIRRORED_REPEAT,
	MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE,
	Repeat = GL_REPEAT,
};

enum class ColorSpace {
	R = GL_RED,
	RG = GL_RG,
	RGB = GL_RGB,
	RGBA = GL_RGBA,

	// srgb formats
	SRGB = GL_SRGB,
	SRGBA = GL_SRGB_ALPHA,
};

enum class InternFormat
{
	R8 = GL_R8,
	R16 = GL_R16,
	R32F = GL_R32F,
	R32I = GL_R32I,
	R32UI = GL_R32UI,

	RG8 = GL_RG8,
	RG16 = GL_RG16,
	RG32F = GL_RG32F,
	RG32I = GL_RG32I,
	RG32UI = GL_RG32UI,

	RGB8 = GL_RGB8,
	RGB16 = GL_RGB16,
	RGB32F = GL_RGB32F,
	RGB32I = GL_RGB32I,
	RGB32UI = GL_RGB32UI,

	RGBA8 = GL_RGBA8,
	RGBA16 = GL_RGBA16,
	RGBA32F = GL_RGBA32F,
	RGBA32I = GL_RGBA32I,
	RGBA32UI = GL_RGBA32UI,

	// srgb formats
	SRGB8 = GL_SRGB8,
	SRGBA8 = GL_SRGB8_ALPHA8,
};

enum class PixelDataType
{
	FLOAT = GL_FLOAT,
	UBYTE = GL_UNSIGNED_BYTE,
	UINT = GL_UNSIGNED_INT,
};

enum class TextureTarget
{
	//1D
	TEXTURE1D = GL_TEXTURE_1D,
	TEXTURE1D_ARRAY = GL_TEXTURE_1D_ARRAY,

	//2D
	TEXTURE2D = GL_TEXTURE_2D,

	// 3D
	TEXTURE2D_ARRAY = GL_TEXTURE_2D_ARRAY,
	TEXTURE3D = GL_TEXTURE_3D,

	// cubemap
	CUBE_POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	CUBE_NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	CUBE_POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	CUBE_NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	CUBE_POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	CUBE_NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

namespace nex::Texture
{
	bool isCubeTarget(TextureTarget target);
}


struct TextureData
{
	TextureFilter minFilter;  // minification filter
	TextureFilter magFilter;  // magnification filter
	TextureUVTechnique uvTechnique;
	ColorSpace colorspace;
	PixelDataType pixelDataType;
	InternFormat internalFormat;
	bool generateMipMaps;
};


class TextureGL
{
public:
	explicit TextureGL();
	TextureGL(GLuint texture);
	TextureGL(TextureGL&& o) noexcept;
	TextureGL& operator=(TextureGL&& o) noexcept;


	TextureGL(const TextureGL&) = delete;
	TextureGL& operator=(const TextureGL&) = delete;

	virtual ~TextureGL();


	/**
	 * Creates a texture from an image store.
	 * The returned texture has to be released by the caller!
	 * NOTE: Supports only TEXTURE2D and CUBEMAP as targets!
	 *
	 * @return a TextureGL or an CubeMapGL dependent on the state of isCubeMap
	 */
	static TextureGL* createFromImage(const StoreImageGL& store, const TextureData& data, bool isCubeMap);

	GLuint getTexture() const;


	unsigned getHeight() const;
	unsigned getWidth() const;

	void setTexture(GLuint id);

	static GLuint getFormat(int numberComponents);

	virtual void release();

	void setHeight(int height);
	void setWidth(int width);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	friend RenderTargetGL;
	friend CubeRenderTargetGL;
	friend BaseRenderTargetGL;
	GLuint textureID;
	int width;
	int height;
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
		POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};

	explicit CubeMapGL();
	CubeMapGL(GLuint cubeMap);
	CubeMapGL(CubeMapGL&& o) = default;
	CubeMapGL& operator=(CubeMapGL&& o) = default;

	CubeMapGL(const CubeMapGL& other) = delete;
	CubeMapGL& operator=(const CubeMapGL& other) = delete;

	virtual ~CubeMapGL() = default;

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
	CubeRenderTargetGL& operator=(CubeRenderTargetGL&&) = delete;


	CubeRenderTargetGL(const CubeRenderTargetGL&) = delete;
	CubeRenderTargetGL& operator=(const CubeRenderTargetGL&) = delete;
	
	virtual ~CubeRenderTargetGL();



	CubeMapGL* createCopy();

	GLuint getRenderBuffer();
	GLuint getCubeMapGL();

	CubeMapGL* getCubeMap();

	inline int getHeightMipLevel(unsigned int mipMapLevel) const {
		return (int)(height * std::pow(0.5, mipMapLevel));
	}

	inline int getWidthMipLevel(unsigned int mipMapLevel) const {
		return (int)(height * std::pow(0.5, mipMapLevel));
	}

	void release();

	void resizeForMipMap(unsigned int mipMapLevel);

	void setRenderBuffer(GLuint newValue);
	void setCubeMap(CubeMapGL* cubeMap);
	void setCubeMapResult(GLuint newValue);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	//friend CubeRenderTargetGL;
	GLuint renderBuffer;
	CubeMapGL* cubeMapResult;
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

	static RenderTargetGL* createMultisampled(int width, int height, const TextureData& data,
		GLuint samples, GLuint depthStencilType);

	static RenderTargetGL* createSingleSampled(int width, int height, const TextureData& data, GLuint depthStencilType);

	static RenderTargetGL* createVSM(int width, int height);

	GLuint getRenderBuffer();
	GLuint getTextureGL();
	TextureGL* getTexture();

	void release();

	void setRenderBuffer(GLuint newValue);

	void setTexture(TextureGL* texture);
	void setTextureBuffer(GLuint newValue);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	friend CubeRenderTargetGL;
	nex::Guard<TextureGL> textureBuffer;
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