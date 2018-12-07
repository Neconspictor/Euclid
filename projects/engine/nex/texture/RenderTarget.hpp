#pragma once
#include <nex/util/Math.hpp>
#include <nex/util/Memory.hpp>

namespace nex
{

	enum class DepthStencil;
	class RenderBuffer;
	class Texture;
	struct TextureData;

	// Has to be implemented by the renderer backend
	class RenderTargetImpl
	{
	public:
		// Class and subclasses shouldn't be movable/copiable
		// Implicitly removes auto-generated move constructor/assignment operator
		// Inherited classes cannot be copied/moved as well
		RenderTargetImpl(const RenderTargetImpl&) = delete;
		RenderTargetImpl& operator=(const RenderTargetImpl&) = delete;

		// virtual needed for backend implementations
		virtual ~RenderTargetImpl() = default;

		Texture* getTexture();

		/**
		 * Sets the texture of this render target.
		 * The previous texture won't be destroyed, so you have to delete it manually
		 *   (use getTexture and than delete it).
		 * NOTE: You should know what you do, when setting the texture manually!
		 */
		void setTexture(Texture* texture);

	protected:
		RenderTargetImpl(unsigned width, unsigned height);
		Guard<Texture> mRenderResult;
	};


	class RenderTarget
	{
	public:
		RenderTarget(const RenderTarget& other) = delete;
		RenderTarget& operator=(const RenderTarget& other) = delete;

		// Has to be implemented by renderer backend
		RenderTarget(RenderTargetImpl* impl);


		virtual ~RenderTarget();

		// Has to be implemented by renderer backend
		void bind();

		// Has to be implemented by renderer backend
		void copyFrom(RenderTarget* dest, const Dimension& sourceDim, int components);

		// Has to be implemented by renderer backend
		void unbind();

		// Has to be implemented by renderer backend
		static RenderTarget* createMultisampled(int width, int height, const TextureData& data,
			unsigned samples, DepthStencil depthStencilType);

		// Has to be implemented by renderer backend
		static RenderTarget* createSingleSampled(int width, int height, const TextureData& data, DepthStencil depthStencilType);

		// Has to be implemented by renderer backend
		static RenderTarget* createVSM(int width, int height);

		//void copyFrom(BaseRenderTarget* dest, const Dimension& sourceDim, int components);

		// Has to be implemented by renderer backend
		int getHeight() const;

		RenderTargetImpl* getImpl() const;

		Texture* getTexture();

		// Has to be implemented by renderer backend
		int getWidth() const;

		/**
		 * Sets the texture of this render target. 
		 * The previous texture won't be destroyed, so you have to delete it manually
		 *   (use getTexture and than delete it).
		 * NOTE: You should know what you do, when setting the texture manually!
		 * NOTE: Has to be implemented by renderer backend
		 */
		void setTexture(Texture* texture);

	protected:

		RenderTargetImpl* mImpl;
	};


	class CubeRenderTarget : public RenderTarget
	{
	public:

		//CubeMap* createCopy();

		// Has to be implemented by renderer backend
		//TODO depthStencilType isn't used currently
		static CubeRenderTarget* createSingleSampled(int width, int height, const TextureData& data, DepthStencil depthStencilType);

		int getHeightMipLevel(unsigned int mipMapLevel) const;

		int getWidthMipLevel(unsigned int mipMapLevel) const;

		// Has to be implemented by renderer backend
		void resizeForMipMap(unsigned int mipMapLevel);

	protected:
		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		CubeRenderTarget(int width, int height, TextureData data);
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


	class DepthMap : public RenderTarget
	{
	public:
		// Has to be implemented by renderer backend
		static DepthMap* create(unsigned width, unsigned height);

	protected:

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		DepthMap(int width, int height);
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
		Texture* getPosition() const;
		// Has to be implemented by renderer backend
		RenderBuffer* getDepth() const;
	};
}