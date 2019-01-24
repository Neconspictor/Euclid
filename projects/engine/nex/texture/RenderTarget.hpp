#pragma once
#include <nex/util/Math.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
	class RenderBuffer;
	class Texture;
	class RenderTarget;
	struct TextureData;

	// Has to be implemented by the renderer backend
	class RenderTargetImpl
	{
	public:

		RenderTargetImpl() = default;
		// Class and subclasses shouldn't be movable/copiable
		// Implicitly removes auto-generated move constructor/assignment operator
		// Inherited classes cannot be copied/moved as well
		RenderTargetImpl(const RenderTargetImpl&) = delete;
		RenderTargetImpl& operator=(const RenderTargetImpl&) = delete;

		// virtual needed for backend implementations
		virtual ~RenderTargetImpl() = default;
	};


	class RenderTarget
	{
	public:

		// Has to be implemented by renderer backend
		RenderTarget(std::unique_ptr<RenderTargetImpl> impl);

		RenderTarget(const RenderTarget& other) = delete;
		RenderTarget& operator=(const RenderTarget& other) = delete;

		// Has to be implemented by renderer backend
		void bind();

		void clear(int components);

		// Has to be implemented by renderer backend
		void unbind();

		// Has to be implemented by renderer backend
		//static RenderTarget* createVSM(int width, int height);

		/**
		 * Provides access to the used depth-stencil map.
		 * Null will be returned if the render target has no assigned depth-stencil map.
		 */
		Texture* getDepthStencilMap();

		/**
		 * Provides access to the used depth-stencil map.
		 * Null will be returned if the render target has no assigned depth-stencil map.
		 */
		std::shared_ptr<Texture> getDepthStencilMapShared();

		// Has to be implemented by renderer backend
		RenderTargetImpl* getImpl() const;

		// Has to be implemented by renderer backend
		Texture* getRenderResult();

		/**
		 * Sets the texture of this render target and returns the old texture.
		 * NOTE: You should know what you do, when setting the texture manually!
		 * NOTE: Has to be implemented by renderer backend
		 */
		Texture* setRenderResult(Texture* texture);

		// Has to be implemented by renderer backend
		void useDepthStencilMap(std::shared_ptr<Texture> depthStencilMap);

	protected:

		std::unique_ptr<RenderTargetImpl> mImpl;

		// Has to be implemented by renderer backend
		void setImpl(std::unique_ptr<RenderTargetImpl> impl);
	};

	class RenderTarget2D : public RenderTarget
	{
	public:

		RenderTarget2D(std::unique_ptr<RenderTargetImpl> impl);

		//Has to be implemented by renderer backend
		RenderTarget2D(int width, int height, const TextureData& data,
			unsigned samples = 1, std::shared_ptr<Texture> depthStencilMap = nullptr);

		// Has to be implemented by renderer backend
		void blit(RenderTarget2D* dest, const Dimension& sourceDim, int components);

		// Has to be implemented by renderer backend
		//static RenderTarget* createVSM(int width, int height);

		//void copyFrom(BaseRenderTarget* dest, const Dimension& sourceDim, int components);

		unsigned getHeight() const;

		unsigned getWidth() const;
	};


	class CubeRenderTarget : public RenderTarget
	{
	public:

		//CubeMap* createCopy();

		// Has to be implemented by renderer backend
		//TODO depthStencilType isn't used currently
		CubeRenderTarget(int width, int height, TextureData data);

		// Has to be implemented by renderer backend
		unsigned getHeightMipLevel(unsigned int mipMapLevel) const;

		// Has to be implemented by renderer backend
		unsigned getWidthMipLevel(unsigned int mipMapLevel) const;

		// Has to be implemented by renderer backend
		unsigned getSideWidth() const;

		// Has to be implemented by renderer backend
		unsigned getSideHeight() const;

		// Has to be implemented by renderer backend
		void resizeForMipMap(unsigned int mipMapLevel);
	};


	class CubeDepthMap : public RenderTarget
	{
	public:

		// Has to be implemented by renderer backend
		static CubeDepthMap* create(unsigned width, unsigned height);

	protected:

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		CubeDepthMap(int width, int height);
		glm::mat4 matrices[6];
	};

	class PBR_GBuffer : public RenderTarget 
	{
	public:

		// Has to be implemented by renderer backend
		PBR_GBuffer(int width, int height);

		// Has to be implemented by renderer backend
		static PBR_GBuffer* create(unsigned width, unsigned height);

		// Has to be implemented by renderer backend
		Texture* getAlbedo() const;
		// Has to be implemented by renderer backend
		Texture* getAoMetalRoughness() const;
		// Has to be implemented by renderer backend
		Texture* getNormal() const;
		// Has to be implemented by renderer backend
		Texture* getDepth() const;
	};
}