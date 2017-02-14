#pragma once
#include <texture/Texture.hpp>
#include <glad/glad.h>

class RendererOpenGL;

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

	void release();

	void setTexture(GLuint id);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	GLuint textureID;
};

class RenderTargetGL : public RenderTarget
{
public:
	explicit RenderTargetGL();
	virtual ~RenderTargetGL();

	GLuint getFrameBuffer();
	GLuint getRenderBuffer();
	GLuint getTextureGL();
	Texture* getTexture() override;

	void setFrameBuffer(GLuint newValue);
	void setRenderBuffer(GLuint newValue);
	void setTextureBuffer(GLuint newValue);

protected:
	friend RendererOpenGL; // allow the OpenGL renderer easier access
	GLuint frameBuffer;
	TextureGL textureBuffer;
	GLuint renderBuffer;
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