#pragma once
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/opengl/opengl.hpp>

namespace nex
{
	enum class RenderAttachmentType;
	class RenderTargetGL;
	class RenderBackend;
	class CubeRenderTargetGL;


	enum class AttachmentTypeGL
	{
		ATTACHMENT_COLOR = GL_COLOR_ATTACHMENT0,
		ATTACHMENT_DEPTH = GL_DEPTH_ATTACHMENT,
		ATTACHMENT_STENCIL = GL_STENCIL_ATTACHMENT,
		ATTACHMNET_DEPTH_STENCIL = GL_DEPTH_STENCIL_ATTACHMENT,
	};


	GLuint translate(RenderAttachmentType type, unsigned attachIndex);

	class RenderTarget::Impl
	{
	public:

		/**
		 * Creates a RenderTargetGL object, that generates a framebuffer
		 */
		explicit Impl(unsigned width, unsigned height);

		/**
		 * Creates a RenderTargetGL object, that doesn't create a framebuffer, but uses an existing one.
		 */
		explicit Impl(GLuint frameBuffer, unsigned width, unsigned height);

		virtual ~Impl();

		void addColorAttachment(RenderAttachment attachment);

		void assertCompletion() const;

		void bind() const;

		void updateDrawColorAttachmentList() const;

		void enableDrawToColorAttachments(bool enable);

		void enableReadColorAttachment(unsigned index, bool enable);

		void enableDrawColorAttachment(unsigned index, bool enable);

		void updateReadFromColorAttachmentList() const;

		void finalizeColorAttachments() const;

		std::vector<RenderAttachment>& getColorAttachments();

		RenderAttachment& getDepthAttachment();

		GLuint getFrameBuffer() const;

		static unsigned getLayerFromCubeMapSide(CubeMapSide side);

		unsigned getWidth() const;

		unsigned getHeight() const;

		bool isComplete() const;

		static bool isDepthType(RenderAttachmentType type);

		void setFrameBuffer(GLuint newValue);

		void resetAttachments(const std::vector<RenderAttachment>& attachments);

		void updateColorAttachment(unsigned index) const;

		void updateDepthAttachment() const;

		void useDepthAttachment(RenderAttachment attachment);

	protected:

		friend RenderTarget;

		GLuint mFrameBuffer;
		std::vector<RenderAttachment> mColorAttachments;
		std::vector<bool> mColorAttachmentReadStatus;
		std::vector<bool> mColorAttachmentDrawStatus;
		std::unique_ptr<RenderAttachment> mDepthAttachment;
		unsigned mWidth;
		unsigned mHeight;


		const RenderAttachment* getByIndex(const unsigned colorAttachIndex) const;
		static bool isArrayTarget(GLenum textureTarget);
		void updateAttachment(const RenderAttachment& attachment) const;

		std::vector<GLenum> calcEnabledReadColorAttachments() const;
		std::vector<GLenum> calcEnabledDrawColorAttachments() const;

		void init();

		/**
		 * This function binds the specified framebuffer and reverse it again, so that the OpenGL state remains unchanged.
		 * The purpose of this function is, that a framebuffer object gets created (certain opengl function are not working correctly otherwise). 
		 */
		static void bindOnce(GLuint frameBufferID);
	};

	class RenderTarget2DGL : public RenderTarget::Impl
	{
	public:

		explicit RenderTarget2DGL(unsigned width, 
			unsigned height, 
			const TextureDesc& data, 
			unsigned samples);

		explicit RenderTarget2DGL(GLuint frameBuffer, unsigned width,
			unsigned height);

		// Has to be implemented by renderer backend
		void blit(RenderTarget2DGL* dest, const Dimension& sourceDim, GLuint components) const;
		static GLint getRenderComponents(int components);
	};

	
	class CubeRenderTargetGL : public RenderTarget::Impl
	{
	public:
		explicit CubeRenderTargetGL(unsigned width, unsigned height, TextureDesc data, InternFormat depthFormat = InternFormat::DEPTH24);

		void useSide(CubeMapSide side, unsigned mipLevel);

		//nex::CubeMapGL* createCopy();

		int getHeightMipLevel(unsigned mipMapLevel) const {
			return (int)(mHeight * std::pow(0.5, mipMapLevel));
		}

		int getWidthMipLevel(unsigned mipMapLevel) const {
			return (int)(mWidth * std::pow(0.5, mipMapLevel));
		}

		void resizeForMipMap(unsigned mipMapLevel);
	};


	class CubeDepthMapGL : public RenderTarget::Impl
	{
	public:
		explicit CubeDepthMapGL(int width, int height);

		virtual ~CubeDepthMapGL() = default;

	private:
		friend RenderBackend; // allow the OpenGL renderer easier access
		glm::mat4 matrices[6];
	};
}