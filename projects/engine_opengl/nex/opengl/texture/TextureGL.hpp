#pragma once
#include <nex/texture/Texture.hpp>
#include <glad/glad.h>
#include <nex/util/Util.hpp>
#include <nex/texture/TextureManager.hpp>

class RenderTargetGL;
class RendererOpenGL;
class CubeRenderTargetGL;
class BaseRenderTargetGL;


class TextureGL : public Texture
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

	virtual void release();

	void setTexture(GLuint id);


	static GLint mapFilter(TextureFilter filter, bool useMipMaps);
	static GLint mapUVTechnique(TextureUVTechnique technique);
	static GLuint getFormat(ColorSpace colorspace);
	static GLuint getFormat(int numberComponents);
	static GLuint getInternalFormat(GLuint format, bool useSRGB, bool isFloatData, Resolution resolution);
	static GLuint getType(bool isFloatData);

	static GLuint rgba_float_resolutions[3];
	static GLuint rgb_float_resolutions[3];
	static GLuint rg_float_resolutions[3];


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

	virtual void release() override;
};

class CubeMapGL : public CubeMap, public TextureGL
{
public:
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


	void generateMipMaps() override;

	GLuint getCubeMap() const;

	void setCubeMap(GLuint id);

	friend CubeRenderTargetGL;
};

class BaseRenderTargetGL : public virtual BaseRenderTarget {
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

private:
	void swap(BaseRenderTargetGL& o);

protected:
	GLuint frameBuffer;
};

class CubeRenderTargetGL : public CubeRenderTarget, public BaseRenderTargetGL
{
public:
	explicit CubeRenderTargetGL(int width, int height, TextureData data);
	
	CubeRenderTargetGL(CubeRenderTargetGL&&) = default;
	CubeRenderTargetGL& operator=(CubeRenderTargetGL&&) = default;


	CubeRenderTargetGL(const CubeRenderTargetGL&) = delete;
	CubeRenderTargetGL& operator=(const CubeRenderTargetGL&) = delete;
	
	virtual ~CubeRenderTargetGL();



	virtual CubeMap* createCopy() override;

	GLuint getRenderBuffer();
	GLuint getCubeMapGL();
	CubeMap* getCubeMap() override;
	GLuint getRendertargetTexture();

	void release();

	void resizeForMipMap(unsigned int mipMapLevel) override;

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

class RenderTargetGL : public RenderTarget, public BaseRenderTargetGL
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
	Texture* getTexture() override;

	void release();

	void setRenderBuffer(GLuint newValue);
	void setTextureBuffer(GLuint newValue);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	friend CubeRenderTargetGL;
	TextureGL textureBuffer;
	GLuint renderBuffer;
};

class CubeDepthMapGL : public CubeDepthMap, public BaseRenderTargetGL, public TextureGL
{
public:
	explicit CubeDepthMapGL(int width, int height);
	
	CubeDepthMapGL(CubeDepthMapGL&& other) = default;
	CubeDepthMapGL& operator=(CubeDepthMapGL&& other) =default;

	CubeDepthMapGL(const CubeDepthMapGL& other) = delete;
	CubeDepthMapGL& operator=(const CubeDepthMapGL& other) = delete;

	virtual ~CubeDepthMapGL();

	GLuint getCubeMapTexture() const;
	CubeMap* getCubeMap() override;
	GLuint getFramebuffer() const;

	void release();

private:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	CubeMapGL cubeMap;
};

class DepthMapGL : public DepthMap, public BaseRenderTargetGL
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
	Texture* getTexture() override;

	void release();

private:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	TextureGL texture;
};

class PBR_GBufferGL : public PBR_GBuffer, public BaseRenderTargetGL {
public:
	explicit PBR_GBufferGL(int width, int height);
	PBR_GBufferGL(PBR_GBufferGL&& o) = default;
	PBR_GBufferGL& operator=(PBR_GBufferGL&& o) = default;


	PBR_GBufferGL(const PBR_GBufferGL&) = delete;
	PBR_GBufferGL& operator=(const PBR_GBufferGL&) = delete;

	virtual ~PBR_GBufferGL() {}

	virtual Texture* getAlbedo() override;
	virtual Texture* getAoMetalRoughness() override;
	virtual Texture* getNormal() override;
	virtual Texture* getPosition() override;
	virtual Texture* getDepth() override;


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