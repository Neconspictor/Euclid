#pragma once
#include <nex/util/Math.hpp>
#include <nex/util/Memory.hpp>

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

	enum class TextureFilter
	{
		NearestNeighbor, FIRST = NearestNeighbor,
		Linear,
		Near_Mipmap_Near,     // trilinear filtering with double nearest neighbor filtering
		Near_Mipmap_Linear,   // trilinear filtering from nearest neighbor to bilinear filtering
		Linear_Mipmap_Near,   // trilinear filtering from bilinear to nearest neighbor filtering
		Linear_Mipmap_Linear, LAST = Linear_Mipmap_Linear,// trilinear filtering from bilinear to bilinear filtering
	};

	enum class TextureUVTechnique
	{
		ClampToBorder, FIRST = ClampToBorder,
		ClampToEdge,
		MirrorRepeat,
		MirrorClampToEdge,
		Repeat, LAST = Repeat,
	};

	enum class ColorSpace {
		R, FIRST = R,
		RED_INTEGER,
		RG,
		RGB,
		RGBA,

		// srgb formats
		SRGB,
		SRGBA, LAST = SRGBA,
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
		RGBA32F,
		RGBA32I,
		RGBA32UI,

		// srgb formats
		SRGB8,
		SRGBA8, LAST = SRGBA8,
	};

	enum class PixelDataType
	{
		FLOAT, FIRST = FLOAT,
		UBYTE,
		UINT, LAST = UINT,
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

		// 3D
		TEXTURE2D_ARRAY,
		TEXTURE3D,

		// cubemap
		CUBE_MAP,
		CUBE_POSITIVE_X,
		CUBE_NEGATIVE_X,
		CUBE_POSITIVE_Y,
		CUBE_NEGATIVE_Y,
		CUBE_POSITIVE_Z,
		CUBE_NEGATIVE_Z, LAST = CUBE_NEGATIVE_Z,
	};

	bool isCubeTarget(TextureTarget target);

		
	bool isCubeTarget(TextureTarget target);


	enum class DepthStencilFormat
	{
		DEPTH24_STENCIL8, FIRST = DEPTH24_STENCIL8,  // GL_DEPTH24_STENCIL8 GL_FLOAT_32_UNSIGNED_INT_24_8_REV
		DEPTH32F_STENCIL8, //GL_DEPTH32F_STENCIL8
		DEPTH16,
		DEPTH24,
		DEPTH32,
		DEPTH_COMPONENT32F, //GL_DEPTH_COMPONENT32F
		STENCIL8, LAST = STENCIL8,  //GL_STENCIL_INDEX8
	};

	enum class DepthStencilType
	{
		DEPTH, FIRST = DEPTH,
		STENCIL,
		DEPTH_STENCIL, LAST = DEPTH_STENCIL
	};


	struct TextureData
	{
		TextureFilter minFilter = TextureFilter::Linear_Mipmap_Linear;  // minification filter
		TextureFilter magFilter = TextureFilter::Linear;  // magnification filter
		TextureUVTechnique wrapR = TextureUVTechnique::Repeat;
		TextureUVTechnique wrapS = TextureUVTechnique::Repeat;
		TextureUVTechnique wrapT = TextureUVTechnique::Repeat;
		ColorSpace colorspace = ColorSpace::SRGBA;
		PixelDataType pixelDataType = PixelDataType::UBYTE;
		InternFormat internalFormat = InternFormat::RGBA8;
		bool generateMipMaps = false;
		bool useSwizzle = false;
		glm::vec<4, Channel, glm::highp> swizzle = { Channel::RED, Channel::GREEN, Channel::BLUE, Channel::ALPHA};

		TextureData() {}

		TextureData(TextureFilter minFilter, 
			TextureFilter magFilter, 
			TextureUVTechnique wrapR,
			TextureUVTechnique wrapS,
			TextureUVTechnique wrapT,
			ColorSpace colorspace, 
			PixelDataType pixelDataType, 
			InternFormat internalFormat,
			bool generateMipMaps) : minFilter(minFilter),
			                        magFilter(magFilter),
			                        wrapR(wrapR),
									wrapS(wrapS),
									wrapT(wrapT),
			                        colorspace(colorspace),
			                        pixelDataType(pixelDataType),
			                        internalFormat(internalFormat),
									generateMipMaps(generateMipMaps)
		{
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
		Texture(TextureImpl* impl);

		TextureImpl* getImpl() const;


		/**
		 * Creates a texture from an image store.
		 * The returned texture has to be released by the caller!
		 * NOTE: Supports only TEXTURE2D and CUBEMAP as targets!
		 * NOTE: Has to be implemented by renderer backend
		 *
		 * @return a Texture or an CubeMap dependent on the state of isCubeMap
		 */
		static Texture* createFromImage(const StoreImage& store, const TextureData& data, bool isCubeMap);

		// Has to be implemented by renderer backend
		static Texture* create();

		void setImpl(TextureImpl* impl);

	protected:
		nex::Guard<TextureImpl> mImpl;
	};

	class Texture2D : public Texture
	{
	public:

		// creates an unintialized texture2D object. Shouldn't be used by user code.
		// 
		Texture2D(TextureImpl* impl);

		// Has to be implemented by renderer backend
		Texture2D(unsigned width, unsigned height, const TextureData& textureData, const void* data);

		static Texture2D* create(unsigned width, unsigned height, const TextureData& textureData, const void* data);

		/**
		 * Resizes this 2d texture. Note that the current texels will be discarded.
		 * NOTE: Has to be implemented by renderer backend
		 */
		virtual void resize(unsigned width, unsigned height);

	protected:
	};

	class RenderBuffer : public Texture {
	public:

		// Has to be implemented by renderer backend
		static RenderBuffer* create(unsigned width, unsigned height, DepthStencilFormat format);

		// Has to be implemented by renderer backend
		DepthStencilFormat getFormat() const;

	protected:
		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		RenderBuffer(unsigned width, unsigned height, DepthStencilFormat format);

		DepthStencilFormat mFormat;
	};


	class CubeMap : public Texture
	{
	public:

		/**
		 * Specifies the sides of a cubemap in relation to a coordinate system.
		 * E.g. in a right handed coordinate system POSITIVE_X would specifiy the right side.
		 */
		enum class Side {
			POSITIVE_X,
			NEGATIVE_X,
			POSITIVE_Y,
			NEGATIVE_Y,
			POSITIVE_Z,
			NEGATIVE_Z,
		};

		// has to be implemented by the renderer backend
		static CubeMap* create();

		/**
		 *  Generates mipmaps for the current content of this cubemap.
		 *  NOTE: Has to be implemented by renderer backend
		 */
		 // has to be implemented by the renderer backend
		void generateMipMaps();

		/**
		 * Provides a 'look at' view matrix for a specific cubemap side
		 * The returned view matrix is for right handed coordinate systems
		 */
		static const glm::mat4& getViewLookAtMatrixRH(Side side);

	protected:

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		CubeMap();

		static glm::mat4 rightSide;
		static glm::mat4 leftSide;
		static glm::mat4 topSide;
		static glm::mat4 bottomSide;
		static glm::mat4 frontSide;
		static glm::mat4 backSide;
	};

	struct DepthStencilDesc
	{
		TextureFilter minFilter = TextureFilter::Linear_Mipmap_Linear;  // minification filter
		TextureFilter magFilter = TextureFilter::Linear;  // magnification filter
		TextureUVTechnique wrap = TextureUVTechnique::ClampToEdge;
		DepthStencilFormat format = DepthStencilFormat::DEPTH24_STENCIL8;
		glm::vec4 borderColor = glm::vec4(1.0f);

		DepthStencilDesc() {}

		DepthStencilDesc(TextureFilter minFilter,
			TextureFilter magFilter,
			TextureUVTechnique wrap,
			DepthStencilFormat format,
			glm::vec4 borderColor) : minFilter(minFilter),
			magFilter(magFilter),
			wrap(wrap),
			format(format),
			borderColor(borderColor)
		{
		}
	};

	class DepthStencilMap : public Texture
	{
	public:
		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		DepthStencilMap(int width, int height, const DepthStencilDesc& desc);


		// Has to be implemented by renderer backend
		DepthStencilFormat getFormat();
	};
}