#pragma once
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/util/Memory.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
	class RenderTargetGL;
	class RendererOpenGL;
	class CubeRenderTargetGL;


	enum AttachmentTypeGL
	{
		ATTACHMENT_COLOR = GL_COLOR_ATTACHMENT0,
		ATTACHMENT_DEPTH = GL_DEPTH_ATTACHMENT,
		ATTACHMENT_STENCIL = GL_STENCIL_ATTACHMENT,
		ATTACHMNET_DEPTH_STENCIL = GL_DEPTH_STENCIL_ATTACHMENT,
	};


	GLuint translate(RenderAttachment::Type type, unsigned attachIndex);

	class RenderTargetGL : public RenderTargetImpl
	{
	public:

		/**
		 * Creates a RenderTargetGL object, that generates a framebuffer
		 */
		explicit RenderTargetGL(unsigned width, unsigned height, std::shared_ptr<Texture> depthStencilMap);

		/**
		 * Creates a RenderTargetGL object, that doesn't create a framebuffer, but uses an existing one.
		 */
		explicit RenderTargetGL(unsigned width, unsigned height, GLuint frameBuffer,
			std::shared_ptr<Texture> depthStencilMap);

		virtual ~RenderTargetGL();

		void addAttachment(RenderAttachment attachment);

		void bind();

		Texture* getDepthStencilMap() const;

		std::shared_ptr<Texture> getDepthStencilMapShared() const;

		GLuint getFrameBuffer() const;

		Texture* getResult() const;

		unsigned getWidth() const;

		unsigned getHeight() const;

		bool isComplete() const;

		void setFrameBuffer(GLuint newValue);

		Texture* setRenderResult(Texture* texture);

		void unbind();

		void useDepthStencilMap(std::shared_ptr<Texture> depthStencilMap);
		void updateAttachments();

	protected:

		friend RenderTargetImpl;

		GLuint mFrameBuffer;
		std::unique_ptr<Texture> mRenderResult;
		std::vector<RenderAttachment> mAttachments;
		std::shared_ptr<Texture> mDepthStencilMap;
		unsigned mWidth;
		unsigned mHeight;

		static void validateDepthStencilMap(Texture* texture);
	};

	class RenderTarget2DGL : public RenderTargetGL
	{
	public:

		RenderTarget2DGL(unsigned width, unsigned height, GLuint frameBuffer, std::shared_ptr<Texture> depthStencilMap);

		explicit RenderTarget2DGL(unsigned width, 
			unsigned height, 
			const TextureData& data, 
			unsigned samples, 
			std::shared_ptr<Texture> depthStencilMap);

		// Has to be implemented by renderer backend
		void blit(RenderTarget2DGL* dest, const Dimension& sourceDim, GLuint components);
		static GLint getRenderComponents(int components);
	};

	
	class CubeRenderTargetGL : public RenderTargetGL
	{
	public:
		explicit CubeRenderTargetGL(unsigned width, unsigned height, TextureData data);

		void useSide(CubeMap::Side side, unsigned mipLevel);

		nex::CubeMapGL* createCopy();

		int getHeightMipLevel(unsigned mipMapLevel) const {
			return (int)(mHeight * std::pow(0.5, mipMapLevel));
		}

		int getWidthMipLevel(unsigned mipMapLevel) const {
			return (int)(mWidth * std::pow(0.5, mipMapLevel));
		}

		void resizeForMipMap(unsigned mipMapLevel);

	protected:
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
}