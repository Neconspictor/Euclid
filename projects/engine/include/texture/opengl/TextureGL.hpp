#pragma once
#include <texture/Texture.hpp>
#include <glad/glad.h>
#include <platform/util/Util.hpp>

class RenderTargetGL;
class RendererOpenGL;
class CubeRenderTargetGL;


class TextureGL : public Texture
{
public:
	explicit TextureGL();
	TextureGL(GLuint texture);
	TextureGL(const TextureGL& other);
	TextureGL(TextureGL&& other);
	TextureGL& operator=(const TextureGL& other);
	TextureGL& operator=(TextureGL&& other);

	virtual ~TextureGL();

	GLuint getTexture() const;

	virtual void release();

	void setTexture(GLuint id);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	friend RenderTargetGL;
	friend CubeRenderTargetGL;
	GLuint textureID;
};

class CubeMapGL : public CubeMap, public TextureGL
{
public:
	explicit CubeMapGL();
	CubeMapGL(GLuint cubeMap);
	CubeMapGL(const CubeMapGL& other);
	CubeMapGL(CubeMapGL&& other);
	CubeMapGL& operator=(const CubeMapGL& other);
	CubeMapGL& operator=(CubeMapGL&& other);

	virtual ~CubeMapGL();

	/**
	 * Maps a cube map side to a corresponding opengl view axis 
	 */
	static GLuint mapCubeSideToSystemAxis(Side side);

	GLuint getCubeMap() const;

	void setCubeMap(GLuint id);

	friend CubeRenderTargetGL;
};

class CubeRenderTargetGL : public CubeRenderTarget
{
public:
	explicit CubeRenderTargetGL(int width, int height);
	virtual ~CubeRenderTargetGL();

	virtual CubeMap* createCopy() override;

	GLuint getFrameBuffer();
	GLuint getRenderBuffer();
	GLuint getCubeMapGL();
	CubeMap* getCubeMap() override;
	GLuint getRendertargetTexture();

	void release();

	void setFrameBuffer(GLuint newValue);
	void setRenderBuffer(GLuint newValue);
	void setCubeMapResult(GLuint newValue);
	void setRenderTargetTexture(GLuint newValue);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	friend CubeRenderTargetGL;
	GLuint frameBuffer;
	GLuint renderBuffer;
	GLuint renderTargetTexture;
	CubeMapGL cubeMapResult;
};

class RenderTargetGL : public RenderTarget
{
public:
	explicit RenderTargetGL(int width, int height);
	virtual ~RenderTargetGL();

	void copyFrom(RenderTargetGL* dest, const Dimension& sourceDim, const Dimension& destDim);

	static RenderTargetGL createMultisampled(GLint textureChannel, int width, int height,
		GLuint samples, GLuint depthStencilType);

	static RenderTargetGL createSingleSampled(GLint internalFormat, int width, int height, GLint format, GLint dataType, GLuint depthStencilType);

	static RenderTargetGL createVSM(int width, int height);

	GLuint getFrameBuffer();
	GLuint getRenderBuffer();
	GLuint getTextureGL();
	Texture* getTexture() override;

	void release();

	void setFrameBuffer(GLuint newValue);
	void setRenderBuffer(GLuint newValue);
	void setTextureBuffer(GLuint newValue);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	friend CubeRenderTargetGL;
	GLuint frameBuffer;
	TextureGL textureBuffer;
	GLuint renderBuffer;
};

class CubeDepthMapGL : public CubeDepthMap, public TextureGL
{
public:
	explicit CubeDepthMapGL(int width, int height);
	CubeDepthMapGL(const CubeDepthMapGL& other);
	CubeDepthMapGL(CubeDepthMapGL&& other);

	CubeDepthMapGL& operator=(const CubeDepthMapGL& other);
	CubeDepthMapGL& operator=(CubeDepthMapGL&& other);

	virtual ~CubeDepthMapGL();

	GLuint getCubeMapTexture() const;
	CubeMap* getCubeMap() override;
	GLuint getFramebuffer() const;

	void release();

private:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	CubeMapGL cubeMap;
	GLuint frameBuffer;
};

class DepthMapGL : public DepthMap
{
public:
	explicit DepthMapGL(int width, int height);
	DepthMapGL(const DepthMapGL& other);
	DepthMapGL(DepthMapGL&& other);

	DepthMapGL& operator=(const DepthMapGL& other);
	DepthMapGL& operator=(DepthMapGL&& other);

	virtual ~DepthMapGL();

	GLuint getFramebuffer() const;
	GLuint getTexture() const;
	Texture* getTexture() override;

	void release();

private:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	TextureGL texture;
	GLuint frameBuffer;
};