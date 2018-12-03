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
		explicit RenderTargetGL(int width, int height);
		virtual ~RenderTargetGL();

		void copyFrom(RenderTargetGL* dest, const Dimension& sourceDim, int components);

		static RenderTargetGL* createMultisampled(int width, int height, const TextureData& data,
			GLuint samples, GLuint depthStencilType);

		static RenderTargetGL* createSingleSampled(int width, int height, const TextureData& data, GLuint depthStencilType);

		static RenderTargetGL* createVSM(int width, int height);

		GLuint getRenderBuffer();
		TextureGL* getTexture();

		virtual void release();

		void setRenderBuffer(GLuint newValue);

		void setTexture(TextureGL* texture);
		void setTextureBuffer(GLuint newValue);

	protected:
		friend RendererOpenGL; // allow the OpenGL renderer easier access
		nex::Guard<TextureGL> textureBuffer;
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

		TextureGL* getAlbedo();
		TextureGL* getAoMetalRoughness();
		TextureGL* getNormal();
		TextureGL* getPosition();
		TextureGL* getDepth();


	protected:
		nex::Guard<TextureGL> albedo;
		nex::Guard<TextureGL> aoMetalRoughness;
		nex::Guard<TextureGL> normal;
		nex::Guard<TextureGL> position;
		nex::Guard<RenderBufferGL> depth;
	};

	class OneTextureRenderTarget : public RenderTargetGL {
	public:
		OneTextureRenderTarget(GLuint frameBuffer,
			TextureGL texture,
			unsigned int width,
			unsigned int height);

		virtual ~OneTextureRenderTarget() = default;
	};

}