#pragma once
#include <boost/locale/format.hpp>
#include <nex/texture/TextureSamplerData.hpp>
#include <nex/resource/Resource.hpp>

namespace nex
{
	struct StoreImage;
	class Texture;
	class Sampler;


	class Texture : public Resource
	{
	public:
		class Impl;

		virtual ~Texture();

		
		// Has to be implemented by renderer backend
		/** Creates a new Texture object using a given internal implementation.
		 *  Note: Be aware, that there aren't any checks. So be sure that the implementation is indeed compatible!
		 */
		Texture(std::unique_ptr<Impl> impl);

		Impl* getImpl() const;

		unsigned getLevelZeroMipMapTextureSize();
		static unsigned getMipMapCount(unsigned levelZeroMipMapTextureSize);


		/**
		 * Creates a texture from an image store.
		 * The returned texture has to be released by the caller!
		 * NOTE: Supports only TEXTURE2D and CUBEMAP as targets!
		 * Note: The TextureData members minLOD, maxLOD, lodBaseLevel and lodMaxLevel are not used from the parameter data but inferred from the store image.
		 *		  The resulting lodBaseLevel will start at index 0 and end at store.mipmapCount - 1.
		 * NOTE: Has to be implemented by renderer backend
		 *
		 * @return a Texture or an CubeMap dependent on the state of isCubeMap
		 */
		static Texture* createFromImage(const StoreImage& store, const TextureDesc& data);

		static std::unique_ptr<Texture> createView(Texture* original,
			TextureTarget target,
			unsigned startLevel, 
			unsigned numLevel, 
			unsigned startLayer, 
			unsigned numLayers,
			const TextureDesc& data);

		/**
		 *  Generates mipmaps for the current content of this texture.
		 *  NOTE: Has to be implemented by renderer backend
		 */
		void generateMipMaps();

		// Has to be implemented by renderer backend
		TextureTarget getTarget() const;

		const TextureDesc& getTextureData() const;

		/**
		 * Provides the width of the texture.
		 * For Cubemaps the width of each side is meant.
		 */
		unsigned getWidth() const;

		/**
		 * Provides the height of the texture.
		 * For Cubemaps the height of each side is meant.
		 * If the texture has no height (1D textures), 0 will be returned.
		 */
		unsigned getHeight() const;

		/**
		 * Provides the depth of the texture.
		 * If the texture has no ´depth (1D/2D and cubemap textures), 0 will be returned.
		 */
		unsigned getDepth() const;


		/**
		 * Reads a texture back from the gpu
		 * @param dest : Memory for storing the texture read back from the gpu. Has to be large enough to store the requested texture.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void readback(unsigned mipmapLevel, ColorSpace format, PixelDataType type, void* dest, size_t destBufferSize);

		void setImpl(std::unique_ptr<Impl> impl);

		// Functions for bindless textures
		uint64_t getHandle();
		uint64_t getHandleWithSampler(const Sampler& sampler);
		void residentHandle(uint64_t handle);
		void makeHandleNonResident(uint64_t handle);

	protected:
		std::unique_ptr<Impl> mImpl;
	};


	class Texture1D : public Texture 
	{
	public:
		Texture1D(std::unique_ptr<Impl> impl);
		Texture1D(unsigned width, const TextureDesc& textureData, const void* data);
		virtual ~Texture1D() = default;
		void resize(unsigned width, unsigned mipmapCount, bool autoMipMapCount);
	};

	class Texture2D : public Texture
	{
	public:

		/** Creates a new Texture2D object using a given internal implementation.
		 *  Note: Be aware, that there aren't any checks. So be sure that the implementation is indeed compatible!
		 */
		Texture2D(std::unique_ptr<Impl> impl);

		// Has to be implemented by renderer backend
		Texture2D(unsigned width, unsigned height, const TextureDesc& textureData, const void* data);

		virtual ~Texture2D() = default;

		/**
		 * Resizes this 2d texture. Note that the current texels will be discarded.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void resize(unsigned width, unsigned height, unsigned mipmapCount, bool autoMipMapCount);
	};

	class Texture2DMultisample : public Texture2D
	{
	public:

		/** Creates a new Texture2DMultisample object using a given internal implementation.
		 *  Note: Be aware, that there aren't any checks. So be sure that the implementation is indeed compatible!
		 */
		Texture2DMultisample(std::unique_ptr<Impl> impl);

		// Has to be implemented by renderer backend
		Texture2DMultisample(unsigned width, unsigned height, const TextureDesc& textureData, unsigned samples);

		virtual ~Texture2DMultisample() = default;

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

		/** Creates a new Texture2DArray object using a given internal implementation.
		 *  Note: Be aware, that there aren't any checks. So be sure that the implementation is indeed compatible!
		 */
		Texture2DArray(std::unique_ptr<Impl> impl);

		// Has to be implemented by renderer backend
		Texture2DArray(unsigned width, unsigned height, unsigned depth, const TextureDesc& textureData, const void* data);

		virtual ~Texture2DArray() = default;

		/**
		 * Resizes this 2d texture. Note that the current texels will be discarded.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void resize(unsigned width, unsigned height, unsigned depth, unsigned mipmapCount, bool autoMipMapCount);
	};

	class Texture3D : public Texture
	{
	public:

		/** Creates a new Texture3D object using a given internal implementation.
		 *  Note: Be aware, that there aren't any checks. So be sure that the implementation is indeed compatible!
		 */
		Texture3D(std::unique_ptr<Impl> impl);

		// Has to be implemented by renderer backend
		Texture3D(unsigned width, unsigned height, unsigned depth, const TextureDesc& textureData, const void* data);

		virtual ~Texture3D() = default;

		/**
		 * Resizes this 3d texture. Note that the current texels will be discarded.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void resize(unsigned width, unsigned height, unsigned depth, unsigned mipmapCount, bool autoMipMapCount);
	};

	class RenderBuffer : public Texture {
	public:

		// Has to be implemented by renderer backend
		RenderBuffer(unsigned width, unsigned height, const TextureDesc& data);

		virtual ~RenderBuffer() = default;

		// Has to be implemented by renderer backend
		InternalFormat getFormat() const;
	};


	class CubeMap : public Texture
	{
	public:

		/** Creates a new CubeMap object using a given internal implementation.
		 *  Note: Be aware, that there aren't any checks. So be sure that the implementation is indeed compatible!
		 */
		CubeMap(std::unique_ptr<Impl> impl);

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		CubeMap(unsigned sideWidth, unsigned sideHeight, const TextureDesc& data);

		virtual ~CubeMap() = default;

		unsigned getLayerFaces() { return 6; };

		/**
		 * Provides a 'look at' view matrix for a specific cubemap side
		 */
		static const glm::mat4& getViewLookAtMatrix(CubeMapSide side);

		static const std::vector<glm::mat4>& getViewLookAts();

	protected:

		static std::vector<glm::mat4> mViewLookAts;
	};

	class CubeMapArray : public Texture
	{
	public:

		/** Creates a new CubeMapArray object using a given internal implementation.
		 *  Note: Be aware, that there aren't any checks. So be sure that the implementation is indeed compatible! 
		 */
		CubeMapArray(std::unique_ptr<Impl> impl);

		// Has to be implemented by renderer backend
		CubeMapArray(unsigned sideWidth, unsigned sideHeight, unsigned depth, const TextureDesc& textureData, const void* data);

		virtual ~CubeMapArray() = default;

		void fill(unsigned xOffset, unsigned yOffset, unsigned zOffset,
			unsigned sideWidth, unsigned sideHeight, unsigned layerFaces,
			unsigned mipmapIndex,
			const void* data);

		unsigned getLayerFaces();

		/**
		 * Provides the layer face start index of a cubemap having a given array index.
		 */
		static unsigned getLayerFaceIndex(unsigned arrayIndex);

		/**
		 * Resizes this array texture. Note that the current texels will be discarded.
		 * NOTE: Has to be implemented by renderer backend
		 */
		void resize(unsigned sideWidth, unsigned sideHeight, unsigned depth, unsigned mipmapCount, bool autoMipMapCount);
	};
}