#pragma once
#include <nex/util/Math.hpp>

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
	};


	class RenderTarget
	{
	public:
		RenderTarget(const RenderTarget& other) = delete;
		RenderTarget& operator=(const RenderTarget& other) = delete;


		virtual ~RenderTarget();;

		// Has to be implemented by renderer backend
		static RenderTarget* createMultisampled(int width, int height, const TextureData& data,
			unsigned samples, DepthStencil depthStencilType);

		// Has to be implemented by renderer backend
		static RenderTarget* createSingleSampled(int width, int height, const TextureData& data, DepthStencil depthStencilType);

		// Has to be implemented by renderer backend
		static RenderTarget* createVSM(int width, int height);

		//void copyFrom(BaseRenderTarget* dest, const Dimension& sourceDim, int components);

		Texture* getTexture();

		int getHeight() const;

		int getWidth() const;

		void setTexture(Texture* texture);

	protected:
		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		RenderTarget(RenderTargetImpl* impl, Texture* renderResult);

		Texture* mRenderResult;
		RenderTargetImpl* mImpl;
	};


	class CubeRenderTarget : public RenderTarget
	{
	public:

		//CubeMap* createCopy();

		// Has to be implemented by renderer backend
		static RenderTarget* createSingleSampled(int width, int height, const TextureData& data, DepthStencil depthStencilType);

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
		// Has to be implemented by renderer backend
		static DepthMap* create(unsigned width, unsigned height);

	protected:

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		DepthMap(int width, int height);
	};

	class PBR_GBufferGL : public RenderTarget 
	{
	public:

		// Has to be implemented by renderer backend
		static PBR_GBufferGL* create(unsigned width, unsigned height);

		Texture* getAlbedo() const { return albedo; }
		Texture* getAoMetalRoughness() const { return aoMetalRoughness; }
		Texture* getNormal() const { return normal; }
		Texture* getPosition() const { return position; }
		RenderBuffer* getDepth() const { return depth; }

	protected:

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		PBR_GBufferGL(int width, int height);


		Texture* albedo;
		Texture* aoMetalRoughness;
		Texture* normal;
		Texture* position;
		RenderBuffer* depth;
	};
}