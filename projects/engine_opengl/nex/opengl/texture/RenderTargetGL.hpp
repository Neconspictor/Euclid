#pragma once
#include "nex/opengl/texture/TextureGL.hpp"
#include <nex/util/Memory.hpp>
#include <nex/texture/RenderTarget.hpp>
#include "nex/texture/Texture.hpp"

namespace nex
{
	class RenderTargetGL;
	class RendererOpenGL;
	class CubeRenderTargetGL;

	class RenderTargetGL : public RenderTargetImpl
	{
	public:
		explicit RenderTargetGL(int width, int height, GLuint frameBuffer = GL_FALSE);
		virtual ~RenderTargetGL();

		GLuint getFrameBuffer() const;
		GLuint getRenderBuffer();

		virtual void release();

		void setFrameBuffer(GLuint newValue);
		void setRenderBuffer(GLuint newValue);

	protected:
		friend RendererOpenGL; // allow the OpenGL renderer easier access
		friend RenderTarget;

		GLuint renderBuffer;
		int width;
		int height;
		GLuint frameBuffer;
	};

	
	class CubeRenderTargetGL : public RenderTargetGL
	{
	public:
		explicit CubeRenderTargetGL(int width, int height, TextureData data);

		virtual ~CubeRenderTargetGL();
		
		nex::CubeMapGL* createCopy();

		inline int getHeightMipLevel(unsigned int mipMapLevel) const {
			return (int)(height * std::pow(0.5, mipMapLevel));
		}

		inline int getWidthMipLevel(unsigned int mipMapLevel) const {
			return (int)(height * std::pow(0.5, mipMapLevel));
		}

		void resizeForMipMap(unsigned int mipMapLevel);

	protected:
		friend RendererOpenGL;
		nex::TextureData data;
	};


	class CubeDepthMapGL : public RenderTargetGL
	{
	public:
		explicit CubeDepthMapGL(int width, int height);

		virtual ~CubeDepthMapGL() = default;

	private:
		friend RendererOpenGL; // allow the OpenGL renderer easier access
		glm::mat4 matrices[6];
	};


	class DepthMapGL : public RenderTargetGL
	{
	public:
		explicit DepthMapGL(int width, int height);

		virtual ~DepthMapGL() = default;

	private:
		friend RendererOpenGL; // allow the OpenGL renderer easier access
		TextureGL texture;
	};

	class PBR_GBufferGL : public RenderTargetGL {
	public:
		explicit PBR_GBufferGL(int width, int height);

		virtual ~PBR_GBufferGL() = default;

		Texture* getAlbedo();
		Texture* getAoMetalRoughness();
		Texture* getNormal();
		Texture* getPosition();
		RenderBuffer* getDepth();


	protected:
		nex::Guard<Texture> albedo;
		nex::Guard<Texture> aoMetalRoughness;
		nex::Guard<Texture> normal;
		nex::Guard<Texture> position;
		nex::Guard<RenderBuffer> depth;
	};
}