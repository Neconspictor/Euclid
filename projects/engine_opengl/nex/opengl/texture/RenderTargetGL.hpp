#pragma once
#include "nex/opengl/texture/TextureGL.hpp"
#include <nex/util/Memory.hpp>

struct StoreImageGL;
class RenderTargetGL;
class RendererOpenGL;
class CubeRenderTargetGL;
class BaseRenderTargetGL;

namespace nex
{
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
	};

	class CubeRenderTargetGL : public BaseRenderTargetGL
	{
	public:
		explicit CubeRenderTargetGL(int width, int height, TextureData data);

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

		void resizeForMipMap(unsigned int mipMapLevel);

		void setRenderBuffer(GLuint newValue);
		void setCubeMap(CubeMapGL* cubeMap);
		void setCubeMapResult(GLuint newValue);

	protected:
		friend RendererOpenGL; // allow the OpenGL renderer easier access
		//friend CubeRenderTargetGL;
		GLuint renderBuffer;
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
		CubeDepthMapGL& operator=(CubeDepthMapGL&& other) = default;

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

}