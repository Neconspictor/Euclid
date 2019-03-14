#pragma once
#include <boost/locale/format.hpp>
#include <nex/texture/TextureSamplerData.hpp>

namespace nex
{
	struct StoreImage;
	class Texture;

	// Has to be implemented by the renderer backend
	class TextureImpl
	{
	public:

		// Class and subclasses shouldn't be movable/copiable
		// Implicitly removes auto-generated move constructor/assignment operator
		// Inherited classes cannot be copied/moved as well
		TextureImpl(const TextureImpl&) = delete;
		TextureImpl& operator=(const TextureImpl&) = delete;


		// virtual needed for backend implementations
		virtual ~TextureImpl() = default;

	protected:

		friend Texture;

		TextureImpl() = default;
	};


	class Texture
	{
	public:

		// Class and subclasses shouldn't be movable/copiable
		// Implicitly removes auto-generated move constructor/assignment operator
		// Inherited classes cannot be copied/moved as well
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		virtual ~Texture() = default; // needed for inheritance

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		Texture(std::unique_ptr<TextureImpl> impl);

		TextureImpl* getImpl() const;
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
		 * Reads a texture back from the gpu
		 * @param dest : Memory for storing the texture read back from the gpu. Has to be large enough to store the requested texture.
		 * @param side: Specifies the cubemap side if the texture target is set to CUBE_MAP
		 * NOTE: Has to be implemented by renderer backend
		 */
		void readback(TextureTarget target, unsigned mipmapLevel, ColorSpace format, PixelDataType type, void* dest, CubeMapSide side = CubeMapSide::POSITIVE_X);

		void setImpl(std::unique_ptr<TextureImpl> impl);

	protected:
		std::unique_ptr<TextureImpl> mImpl;
	};

	class Texture2D : public Texture
	{
	public:

		// creates an unintialized texture2D object. Shouldn't be used by user code.
		// 
		Texture2D(std::unique_ptr<TextureImpl> impl);

		// Has to be implemented by renderer backend
		Texture2D(unsigned width, unsigned height, const TextureData& textureData, const void* data);

		// Has to be implemented by renderer backend
		static Texture2D* create(unsigned width, unsigned height, const TextureData& textureData, const void* data);

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
		Texture2DMultisample(std::unique_ptr<TextureImpl> impl);

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
		Texture2DArray(std::unique_ptr<TextureImpl> impl);

		// Has to be implemented by renderer backend
		Texture2DArray(unsigned width, unsigned height, unsigned size, bool immutableStorage, const TextureData& textureData, const void* data);

		/**
		 * Resizes this 2d texture. Note that the current texels will be discarded.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void resize(unsigned width, unsigned height, unsigned size);

		unsigned getWidth() const;
		unsigned getHeight() const;
		unsigned getSize() const;
	};

	class RenderBuffer : public Texture {
	public:

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		RenderBuffer(unsigned width, unsigned height, InternFormat format);

		// Has to be implemented by renderer backend
		InternFormat getFormat() const;
	};


	class CubeMap : public Texture
	{
	public:

		CubeMap(std::unique_ptr<TextureImpl> impl);

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		CubeMap(unsigned sideWidth, unsigned sideHeight, const TextureData& data);

		/**
		 *  Generates mipmaps for the current content of this cubemap.
		 *  NOTE: Has to be implemented by renderer backend
		 */
		 // has to be implemented by the renderer backend
		void generateMipMaps();

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
