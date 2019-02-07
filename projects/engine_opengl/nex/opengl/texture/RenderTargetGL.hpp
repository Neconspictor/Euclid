#pragma once
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/util/Memory.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
	class RenderTargetGL;
	class RenderBackend;
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
		explicit RenderTargetGL();

		/**
		 * Creates a RenderTargetGL object, that doesn't create a framebuffer, but uses an existing one.
		 */
		explicit RenderTargetGL(GLuint frameBuffer);

		virtual ~RenderTargetGL();

		void addAttachment(RenderAttachment attachment);

		void bind();

		void finalizeAttachments();

		const std::vector<RenderAttachment>& getAttachments();

		GLuint getFrameBuffer() const;

		static unsigned getLayerFromCubeMapSide(CubeMap::Side side);

		bool isComplete() const;

		void setFrameBuffer(GLuint newValue);

		void unbind();

		void updateAttachment(unsigned index);

	protected:

		friend RenderTargetImpl;

		GLuint mFrameBuffer;
		std::vector<RenderAttachment> mAttachments;
	};

	class RenderTarget2DGL : public RenderTargetGL
	{
	public:

		explicit RenderTarget2DGL(unsigned width, 
			unsigned height, 
			const TextureData& data, 
			unsigned samples);

		// Has to be implemented by renderer backend
		void blit(RenderTarget2DGL* dest, const Dimension& sourceDim, GLuint components);
		static GLint getRenderComponents(int components);
	};

	
	class CubeRenderTargetGL : public RenderTargetGL
	{
	public:
		explicit CubeRenderTargetGL(unsigned width, unsigned height, TextureData data);

		void useSide(CubeMap::Side side, unsigned mipLevel);

		//nex::CubeMapGL* createCopy();

		int getHeightMipLevel(unsigned mipMapLevel) const {
			return (int)(mHeight * std::pow(0.5, mipMapLevel));
		}

		int getWidthMipLevel(unsigned mipMapLevel) const {
			return (int)(mWidth * std::pow(0.5, mipMapLevel));
		}

		void resizeForMipMap(unsigned mipMapLevel);

	protected:
		nex::TextureData data;
		unsigned mWidth;
		unsigned mHeight;
	};


	class CubeDepthMapGL : public RenderTargetGL
	{
	public:
		explicit CubeDepthMapGL(int width, int height);

		virtual ~CubeDepthMapGL() = default;

	private:
		friend RenderBackend; // allow the OpenGL renderer easier access
		glm::mat4 matrices[6];
	};
}