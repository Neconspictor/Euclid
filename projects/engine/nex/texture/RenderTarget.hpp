#pragma once
#include <nex/util/Math.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/util/ExceptionHandling.hpp>

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

	struct RenderAttachment
	{
		enum class Type
		{
			COLOR, FIRST = COLOR,
			DEPTH,
			STENCIL, 
			DEPTH_STENCIL, LAST = DEPTH_STENCIL,
		};

		unsigned attachIndex = 0;
		unsigned mipmapLevel = 0;
		unsigned layer = 0; // Specifies the layer of an array texture; Must be zero for non array textures;
		CubeMap::Side side = CubeMap::Side::POSITIVE_X;  // only used when target is TextureTarget::CubeMap
		TextureTarget target = TextureTarget::TEXTURE2D;
		Type type = Type::COLOR;
		std::shared_ptr<Texture> texture;

		RenderAttachment() {}

		RenderAttachment( unsigned attachIndex, unsigned mipmapLevel, unsigned layer, CubeMap::Side side, TextureTarget target, Type type, std::shared_ptr<Texture> texture)
		: 
			attachIndex(attachIndex), 
			mipmapLevel(mipmapLevel),
			layer(layer),
			side(side),
			target(target),
			type(type),
			texture(std::move(texture))
		{}

	};


	class RenderTarget
	{
	public:

		// Has to be implemented by renderer backend
		RenderTarget(std::unique_ptr<RenderTargetImpl> impl);


		// Has to be implemented by renderer backend
		/**
		 * Creates an uncompleted render target with no attachments.
		 * Before it can be used for rendering the user has to add attachments, optionally a depth-stencil buffer
		 * and update the attachments.
		 */
		RenderTarget();

		RenderTarget(const RenderTarget& other) = delete;
		RenderTarget& operator=(const RenderTarget& other) = delete;

		// Has to be implemented by renderer backend
		void addColorAttachment(RenderAttachment attachment);

		void assertCompletion() const;

		// Has to be implemented by renderer backend
		void bind();

		// Has to be implemented by renderer backend
		void clear(int components) const;

		// Has to be implemented by renderer backend
		void enableDrawToColorAttachments(bool enable) const;

		// Has to be implemented by renderer backend
		void enableReadFromColorAttachments(bool enable) const;

		// Has to be implemented by renderer backend
		void finalizeAttachments() const;

		// Has to be implemented by renderer backend
		RenderAttachment* getDepthAttachment();

		// Has to be implemented by renderer backend
		//static RenderTarget* createVSM(int width, int height);

		// Has to be implemented by renderer backend
		RenderTargetImpl* getImpl() const;


		// Has to be implemented by renderer backend
		/**
		 * Checks whether the render target is complete. A render target is complete
		 * if it has attachments, the attachments have been updated and optionally a valid depth-stencil map has been assigned.
		 * Note: A render target has to be completed before it can be used for rendering.
		 */
		bool isComplete() const;

		// Has to be implemented by renderer backend
		void unbind() const;

		// Has to be implemented by renderer backend
		void updateColorAttachment(unsigned index) const;

		void updateDepthAttachment() const;

		// Has to be implemented by renderer backend
		void useDepthAttachment(RenderAttachment attachment) const;

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
			unsigned samples = 1);

		// Has to be implemented by renderer backend
		void blit(RenderTarget2D* dest, const Dimension& sourceDim, int components);

		// Has to be implemented by renderer backend
		//static RenderTarget* createVSM(int width, int height);

		//void copyFrom(BaseRenderTarget* dest, const Dimension& sourceDim, int components);
	};


	class CubeRenderTarget : public RenderTarget
	{
	public:

		//CubeMap* createCopy();

		// Has to be implemented by renderer backend
		//TODO depthStencilType isn't used currently
		CubeRenderTarget(int width, int height, TextureData data);

		void useSide(CubeMap::Side side, unsigned int mipLevel = 0);

		// Has to be implemented by renderer backend
		unsigned getHeightMipLevel(unsigned int mipMapLevel) const;

		// Has to be implemented by renderer backend
		unsigned getWidthMipLevel(unsigned int mipMapLevel) const;

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
}
