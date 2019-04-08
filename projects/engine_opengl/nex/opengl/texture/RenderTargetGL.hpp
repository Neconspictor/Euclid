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
		explicit Impl();

		/**
		 * Creates a RenderTargetGL object, that doesn't create a framebuffer, but uses an existing one.
		 */
		explicit Impl(GLuint frameBuffer);

		virtual ~Impl();

		void addColorAttachment(RenderAttachment attachment);

		void assertCompletion() const;

		void bind() const;

		void enableDrawToColorAttachments(bool enable) const;

		void enableReadFromColorAttachments(bool enable) const;

		void finalizeColorAttachments() const;

		std::vector<RenderAttachment>& getColorAttachments();

		RenderAttachment& getDepthAttachment();

		GLuint getFrameBuffer() const;

		static unsigned getLayerFromCubeMapSide(CubeMapSide side);

		bool isComplete() const;

		static bool isDepthType(RenderAttachmentType type);

		void setFrameBuffer(GLuint newValue);

		void unbind() const;

		void updateColorAttachment(unsigned index) const;

		void updateDepthAttachment() const;

		void useDepthAttachment(RenderAttachment attachment);

	protected:

		friend RenderTarget;

		GLuint mFrameBuffer;
		std::vector<RenderAttachment> mColorAttachments;
		std::unique_ptr<RenderAttachment> mDepthAttachment;

		const RenderAttachment* getByIndex(const unsigned colorAttachIndex) const;
		static bool isArrayTarget(GLenum textureTarget);
		void updateAttachment(const RenderAttachment& attachment) const;

		std::vector<GLenum> calcColorAttachments() const;
	};

	class RenderTarget2DGL : public RenderTarget::Impl
	{
	public:

		explicit RenderTarget2DGL(unsigned width, 
			unsigned height, 
			const TextureData& data, 
			unsigned samples);

		explicit RenderTarget2DGL(GLuint frameBuffer, unsigned width,
			unsigned height);

		// Has to be implemented by renderer backend
		void blit(RenderTarget2DGL* dest, const Dimension& sourceDim, GLuint components) const;
		static GLint getRenderComponents(int components);

		unsigned getWidth() const;
		unsigned getHeight() const;

	private:
		unsigned mHeight;
		unsigned mWidth;
	};

	
	class CubeRenderTargetGL : public RenderTarget::Impl
	{
	public:
		explicit CubeRenderTargetGL(unsigned width, unsigned height, TextureData data, InternFormat depthFormat = InternFormat::DEPTH24);

		void useSide(CubeMapSide side, unsigned mipLevel);

		//nex::CubeMapGL* createCopy();

		int getHeightMipLevel(unsigned mipMapLevel) const {
			return (int)(mHeight * std::pow(0.5, mipMapLevel));
		}

		int getWidthMipLevel(unsigned mipMapLevel) const {
			return (int)(mWidth * std::pow(0.5, mipMapLevel));
		}

		void resizeForMipMap(unsigned mipMapLevel);

		// Has to be implemented by renderer backend
		unsigned getWidth() const 
		{
			return mWidth;
		}

		// Has to be implemented by renderer backend
		unsigned getHeight() const
		{
			return mHeight;
		}

	protected:
		nex::TextureData data;
		unsigned mWidth;
		unsigned mHeight;
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