#pragma once
#include <nex/texture/Sampler.hpp>
#include <boost/locale/format.hpp>

namespace nex
{
	struct StoreImage;
	class Texture;

	enum class TextureAccess
	{
		READ_ONLY, FIRST = READ_ONLY,
		READ_WRITE,
		WRITE_ONLY, LAST = WRITE_ONLY,
	};

	enum class ColorSpace {
		R, FIRST = R,
		RED_INTEGER,
		RG,
		RGB,
		RGBA,

		// srgb formats
		SRGB,
		SRGBA, 
		
		DEPTH,
		STENCIL,
		DEPTH_STENCIL, LAST = DEPTH_STENCIL,
	};

	unsigned getComponents(const ColorSpace colorspace);

	enum class InternFormat
	{
		R8, FIRST = R8,
		R16,
		R16F,
		R32F,
		R32I,
		R32UI,

		RG8,
		RG16,
		RG16F,
		RG32F,
		RG32I,
		RG32UI,

		RGB8,
		RGB16,
		RGB16F,
		RGB32F,
		RGB32I,
		RGB32UI,

		RGBA8,
		RGBA16,
		RGBA16F,
		RGBA16_SNORM,
		RGBA32F,
		RGBA32I,
		RGBA32UI,

		// srgb formats
		SRGB8,
		SRGBA8,

		DEPTH24_STENCIL8,  // GL_DEPTH24_STENCIL8 GL_FLOAT_32_UNSIGNED_INT_24_8_REV
		DEPTH32F_STENCIL8, //GL_DEPTH32F_STENCIL8
		DEPTH16,
		DEPTH24,
		DEPTH32,
		DEPTH_COMPONENT32F, //GL_DEPTH_COMPONENT32F
		STENCIL8, LAST = STENCIL8,  //GL_STENCIL_INDEX8
	};

	enum class PixelDataType
	{
		FLOAT, FIRST = FLOAT,
		UBYTE,
		UINT,

		SHORT,

		UNSIGNED_INT_24_8,
		FLOAT_32_UNSIGNED_INT_24_8_REV,
		UNSIGNED_SHORT,
		UNSIGNED_INT_24,
		UNSIGNED_INT_8, LAST = UNSIGNED_INT_8,
	};

	enum class Channel
	{
		RED, FIRST = RED,
		GREEN,
		BLUE,
		ALPHA, LAST = ALPHA,
	};

	enum class TextureTarget
	{
		//1D
		TEXTURE1D, FIRST = TEXTURE1D,
		TEXTURE1D_ARRAY,

		//2D
		TEXTURE2D,

		TEXTURE2D_MULTISAMPLE,

		// 3D
		TEXTURE2D_ARRAY,
		TEXTURE2D_ARRAY_MULTISAMPLE,
		TEXTURE3D,

		// cubemap
		CUBE_MAP, LAST = CUBE_MAP,
	};

		/**
		 * Specifies the sides of a cubemap in relation to a coordinate system.
		 * E.g. in a right handed coordinate system POSITIVE_X would specifiy the right side.
		 */
	enum class CubeMapSide {
		POSITIVE_X,
		NEGATIVE_X,
		POSITIVE_Y,
		NEGATIVE_Y,
		POSITIVE_Z,
		NEGATIVE_Z,
	};

	struct BaseTextureDesc : public SamplerDesc
	{
		CompareFunction compareFunc = CompareFunction::LESS_EQUAL;
		bool generateMipMaps = false;
		unsigned lodBaseLevel = 0; // index of the lowest defined mipmap level
		unsigned lodMaxLevel = 1000.0f; //index of the highest defined mipmap level
		glm::vec<4, Channel, glm::highp> swizzle = { Channel::RED, Channel::GREEN, Channel::BLUE, Channel::ALPHA };
		unsigned textureIndex = 0;
		bool useSwizzle = false;
	};


	struct TextureData : public BaseTextureDesc
	{
		ColorSpace colorspace = ColorSpace::SRGBA;
		PixelDataType pixelDataType = PixelDataType::UBYTE;
		InternFormat internalFormat = InternFormat::RGBA8;

		TextureData() {}

		TextureData(TextureFilter minFilter, 
			TextureFilter magFilter, 
			TextureUVTechnique wrapR,
			TextureUVTechnique wrapS,
			TextureUVTechnique wrapT,
			ColorSpace colorspace, 
			PixelDataType pixelDataType, 
			InternFormat internalFormat,
			bool generateMipMaps) : 
			                        colorspace(colorspace),
			                        pixelDataType(pixelDataType),
			                        internalFormat(internalFormat)
		{
			this->minFilter = minFilter;
			this->magFilter = magFilter;
			this->wrapR = wrapR;
			this->wrapS = wrapS;
			this->wrapT = wrapT;
			this->generateMipMaps = generateMipMaps;
		}

		static TextureData createImage(TextureFilter minFilter,
			TextureFilter magFilter,
			TextureUVTechnique wrapR,
			TextureUVTechnique wrapS,
			TextureUVTechnique wrapT,
			ColorSpace colorspace,
			PixelDataType pixelDataType,
			InternFormat internalFormat,
			bool generateMipMaps)
		{
			TextureData result;
			result.colorspace = colorspace;
			result.pixelDataType = pixelDataType;
			result.internalFormat = internalFormat;
			result.minFilter = minFilter;
			result.magFilter = magFilter;
			result.wrapS = wrapR;
			result.wrapR = wrapS;
			result.wrapT = wrapT;
			result.generateMipMaps = generateMipMaps;
			return result;
		}

		static TextureData createDepth(CompareFunction compareFunction, 
			ColorSpace colorSpace,
			PixelDataType dataType, 
			InternFormat format)
		{
			TextureData result;
			result.useDepthComparison = true;
			result.compareFunction = compareFunction;
			result.colorspace = colorSpace;
			result.pixelDataType = dataType;
			result.internalFormat = format;
			result.minFilter = result.magFilter = TextureFilter::NearestNeighbor;
			result.wrapS = result.wrapR = result.wrapT = TextureUVTechnique::ClampToEdge;
			return result;
		}
	};


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
