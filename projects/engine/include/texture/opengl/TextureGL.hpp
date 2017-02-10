#pragma once
#include <texture/Texture.hpp>
#include <glad/glad.h>

class TextureGL : public Texture
{
public:
	TextureGL(GLuint texture);
	TextureGL(const TextureGL& other);
	TextureGL(TextureGL&& other);
	TextureGL& operator=(const TextureGL& other);
	TextureGL& operator=(TextureGL&& other);

	virtual ~TextureGL();

	GLuint getTexture() const;

	void setTexture(GLuint id);

private:
	GLuint textureID;
};

class DepthMapGL : public DepthMap
{
public:
	explicit DepthMapGL(int width, int height);
	DepthMapGL(const DepthMapGL& other) = delete;
	DepthMapGL(DepthMapGL&& other) = delete;

	DepthMapGL& operator=(const DepthMapGL& other) = delete;
	DepthMapGL& operator=(DepthMapGL&& other) = delete;

	virtual ~DepthMapGL();

	GLuint getFramebuffer() const;
	GLuint getTexture() const;

private:
	GLuint textureID;
	GLuint frameBuffer;
};