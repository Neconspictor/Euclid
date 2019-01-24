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
		explicit RenderTargetGL(unsigned width, unsigned height, GLuint frameBuffer = GL_FALSE,
			std::shared_ptr<Texture> depthStencilMap = nullptr);

		virtual ~RenderTargetGL();

		void addAttachment(RenderAttachment attachment);

		void bind();

		Texture* getDepthStencilMap() const;

		std::shared_ptr<Texture> getDepthStencilMapShared() const;

		GLuint getFrameBuffer() const;

		Texture* getResult() const;

		unsigned getWidth() const;

		unsigned getHeight() const;

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

		RenderTarget2DGL(unsigned width, unsigned height);

		explicit RenderTarget2DGL(unsigned width, 
			unsigned height, 
			const TextureData& data, 
			unsigned samples, 
			std::shared_ptr<Texture> depthStencilMap = nullptr);

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


	class PBR_GBufferGL : public RenderTargetGL {
	public:
		explicit PBR_GBufferGL(int width, int height);

		virtual ~PBR_GBufferGL() = default;

		Texture2D* getAlbedo() const;
		Texture2D* getAoMetalRoughness() const;
		Texture2D* getNormal() const;
		Texture2D* getDepth() const;


	protected:
		RenderAttachment albedo;
		RenderAttachment aoMetalRoughness;
		RenderAttachment normal;
		RenderAttachment depth;
	};
}