#pragma once
#include <boost/locale/format.hpp>
#include <nex/texture/TextureSamplerData.hpp>

namespace nex
{
	struct StoreImage;
	class Texture;


	class Texture
	{
	public:
		class Impl;

		virtual ~Texture();

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		Texture(std::unique_ptr<Impl> impl);

		Impl* getImpl() const;
		static unsigned getMipMapCount(unsigned levelZeroMipMap);


		/**
		 * Creates a texture from an image store.
		 * The returned texture has to be released by the caller!
		 * NOTE: Supports only TEXTURE2D and CUBEMAP as targets!
		 * NOTE: Has to be implemented by renderer backend
		 *
		 * @return a Texture or an CubeMap dependent on the state of isCubeMap
		 */
		static Texture* createFromImage(const StoreImage& store, const TextureData& data, bool isCubeMap);

		static std::unique_ptr<Texture> createView(Texture* original,
			TextureTarget target,
			unsigned minLevel, 
			unsigned numLevel, 
			unsigned minLayer, 
			unsigned numLayers,
			const TextureData& data);

		/**
		 *  Generates mipmaps for the current content of this texture.
		 *  NOTE: Has to be implemented by renderer backend
		 */
		void generateMipMaps();

		const TextureData& getTextureData() const;

		/**
		 * Reads a texture back from the gpu
		 * @param dest : Memory for storing the texture read back from the gpu. Has to be large enough to store the requested texture.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void readback(unsigned mipmapLevel, ColorSpace format, PixelDataType type, void* dest, size_t destBufferSize);

		void setImpl(std::unique_ptr<Impl> impl);

	protected:
		std::unique_ptr<Impl> mImpl;
	};

	class Texture2D : public Texture
	{
	public:

		// creates an unintialized texture2D object. Shouldn't be used by user code.
		// 
		Texture2D(std::unique_ptr<Impl> impl);

		// Has to be implemented by renderer backend
		Texture2D(unsigned width, unsigned height, const TextureData& textureData, const void* data);

		/**
		 * Resizes this 2d texture. Note that the current texels will be discarded.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void resize(unsigned width, unsigned height);

		// Has to be implemented by renderer backend
		unsigned getWidth() const;

		// Has to be implemented by renderer backend
		unsigned getHeight() const;
	};

	class Texture2DMultisample : public Texture2D
	{
	public:

		// creates an unintialized texture2D object. Shouldn't be used by user code.
		// 
		Texture2DMultisample(std::unique_ptr<Impl> impl);

		// Has to be implemented by renderer backend
		Texture2DMultisample(unsigned width, unsigned height, const TextureData& textureData, unsigned samples);

		/**
		 * Resizes this 2d texture. Note that the current texels will be discarded.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void resize(unsigned width, unsigned height);

		unsigned getSamples() const;
	};

	class Texture2DArray : public Texture
	{
	public:

		// creates an unintialized texture2D object. Shouldn't be used by user code.
		// 
		Texture2DArray(std::unique_ptr<Impl> impl);

		// Has to be implemented by renderer backend
		Texture2DArray(unsigned width, unsigned height, unsigned depth, const TextureData& textureData, const void* data);

		/**
		 * Resizes this 2d texture. Note that the current texels will be discarded.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void resize(unsigned width, unsigned height, unsigned depth);

		unsigned getWidth() const;
		unsigned getHeight() const;
		unsigned getDepth() const;
	};

	class RenderBuffer : public Texture {
	public:

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		RenderBuffer(unsigned width, unsigned height, const TextureData& data);

		// Has to be implemented by renderer backend
		InternFormat getFormat() const;
	};


	class CubeMap : public Texture
	{
	public:

		CubeMap(std::unique_ptr<Impl> impl);

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		CubeMap(unsigned sideWidth, unsigned sideHeight, const TextureData& data);

		unsigned getSideWidth() const;
		unsigned getSideHeight() const;

		/**
		 * Provides a 'look at' view matrix for a specific cubemap side
		 * The returned view matrix is for right handed coordinate systems
		 */
		static const glm::mat4& getViewLookAtMatrixRH(CubeMapSide side);

	protected:

		static glm::mat4 rightSide;
		static glm::mat4 leftSide;
		static glm::mat4 topSide;
		static glm::mat4 bottomSide;
		static glm::mat4 frontSide;
		static glm::mat4 backSide;
	};
}
