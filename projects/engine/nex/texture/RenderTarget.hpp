#pragma once
#include <nex/math/Math.hpp>

namespace nex
{
	class RenderBuffer;
	class Texture;
	class RenderTarget;
	struct TextureDesc;
	enum class CubeMapSide;
	struct TextureDesc;
	struct RenderAttachment;
	class Texture2D;


	class RenderTarget
	{
	public:

		class Impl;

		// Has to be implemented by renderer backend
		RenderTarget(std::unique_ptr<Impl> impl);


		// Has to be implemented by renderer backend
		/**
		 * Creates a render target with no attachments.
		 */
		RenderTarget(unsigned width, unsigned height);

		virtual ~RenderTarget();

		RenderTarget(const RenderTarget& other) = delete;
		RenderTarget& operator=(const RenderTarget& other) = delete;

		// Has to be implemented by renderer backend
		void addColorAttachment(RenderAttachment attachment);

		void assertCompletion() const;

		// Has to be implemented by renderer backend
		void bind();

		// Has to be implemented by renderer backend
		void blit(RenderTarget* dest, const Dimension& sourceDim, int components);

		// Has to be implemented by renderer backend
		void clear(int components) const;

		// Has to be implemented by renderer backend
		void enableDrawToColorAttachments(bool enable) const;

		// Has to be implemented by renderer backend
		void enableDrawToColorAttachment(unsigned index, bool enable);

		// Has to be implemented by renderer backend
		void enableReadFromColorAttachments() const;

		// Has to be implemented by renderer backend
		void finalizeAttachments() const;

		// Has to be implemented by renderer backend
		std::vector<RenderAttachment>& getColorAttachments();

		// Has to be implemented by renderer backend
		const std::vector<RenderAttachment>& getColorAttachments() const;

		Texture* getColorAttachmentTexture(std::size_t attachmentIndex);

		// Has to be implemented by renderer backend
		RenderAttachment* getDepthAttachment() const;

		// Has to be implemented by renderer backend
		//static RenderTarget* createVSM(int width, int height);

		// Has to be implemented by renderer backend
		Impl* getImpl() const;

		// Has to be implemented by renderer backend
		unsigned getWidth() const;

		// Has to be implemented by renderer backend
		unsigned getHeight() const;


		// Has to be implemented by renderer backend
		/**
		 * Checks whether the render target is complete. A render target is complete
		 * if it has attachments, the attachments have been updated and optionally a valid depth-stencil map has been assigned.
		 * Note: A render target has to be completed before it can be used for rendering.
		 */
		bool isComplete() const;

		void resetAttachments(const std::vector<RenderAttachment>& attachments);

		// Has to be implemented by renderer backend
		/**
		 * @param index: The vector index of the color attachment (see getColorAttachments())
		 */
		void updateColorAttachment(unsigned index) const;

		void updateDepthAttachment() const;

		// Has to be implemented by renderer backend
		void useDepthAttachment(RenderAttachment attachment) const;


	protected:

		std::unique_ptr<Impl> mImpl;

		// Has to be implemented by renderer backend
		void setImpl(std::unique_ptr<Impl> impl);
	};

	class RenderTarget2D : public RenderTarget
	{
	public:

		RenderTarget2D(std::unique_ptr<Impl> impl);


		//Has to be implemented by renderer backend
		RenderTarget2D(int width, int height, const TextureDesc& data,
			unsigned samples = 1);

		// Has to be implemented by renderer backend
		//static RenderTarget* createVSM(int width, int height);

		//void copyFrom(BaseRenderTarget* dest, const Dimension& sourceDim, int components);

		// Has to be implemented by renderer backend
		unsigned getWidth() const;

		// Has to be implemented by renderer backend
		unsigned getHeight() const;

		Texture2D* getColor0AttachmentTexture();
	};


	class CubeRenderTarget : public RenderTarget
	{
	public:

		//CubeMap* createCopy();

		// Has to be implemented by renderer backend
		//TODO depthStencilType isn't used currently
		CubeRenderTarget(int width, int height, TextureDesc data);

		void useSide(CubeMapSide side, unsigned int mipLevel = 0);

		// Has to be implemented by renderer backend
		unsigned getHeightMipLevel(unsigned int mipMapLevel) const;

		// Has to be implemented by renderer backend
		unsigned getWidthMipLevel(unsigned int mipMapLevel) const;

		// Has to be implemented by renderer backend
		void resizeForMipMap(unsigned int mipMapLevel);

		// Has to be implemented by renderer backend
		unsigned getWidth() const;

		// Has to be implemented by renderer backend
		unsigned getHeight() const;
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


	class RenderTargetSwitcher {
	public:

		RenderTargetSwitcher(RenderTarget* target,
			unsigned colorAttachIndex,
			std::shared_ptr<Texture> first,
			std::shared_ptr<Texture> second);

		void switchTexture(bool update = true);
		bool getActive() const;
		const std::vector<std::shared_ptr<Texture>>& getTextures() const;
		void setActive(bool active, bool update = true);

		void setTarget(RenderTarget* target, bool update = true);

		void setTextures(std::shared_ptr<Texture> first,
			std::shared_ptr<Texture> second, bool update = true);

		void updateRenderTarget();

	private:

		RenderTarget* mTarget;
		std::vector<std::shared_ptr<Texture>> mTextures;
		bool mActive;
		unsigned mColorAttachIndex;
	};
}