#pragma once
#include <nex/util/Math.hpp>

namespace nex
{
	struct StoreImage;

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
		RG,
		RGB,
		RGBA,

		// srgb formats
		SRGB,
		SRGBA, LAST = SRGBA,
	};

	enum class InternFormat
	{
		R8, FIRST = R8,
		R16,
		R32F,
		R32I,
		R32UI,

		RG8,
		RG16,
		RG32F,
		RG32I,
		RG32UI,

		RGB8,
		RGB16,
		RGB32F,
		RGB32I,
		RGB32UI,

		RGBA8,
		RGBA16,
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
		CUBE_POSITIVE_X,
		CUBE_NEGATIVE_X,
		CUBE_POSITIVE_Y,
		CUBE_NEGATIVE_Y,
		CUBE_POSITIVE_Z,
		CUBE_NEGATIVE_Z, LAST = CUBE_NEGATIVE_Z,
	};

	enum class DepthStencil
	{

	};
		
	bool isCubeTarget(TextureTarget target);


	struct TextureData
	{
		TextureFilter minFilter;  // minification filter
		TextureFilter magFilter;  // magnification filter
		TextureUVTechnique uvTechnique;
		ColorSpace colorspace;
		PixelDataType pixelDataType;
		InternFormat internalFormat;
		bool generateMipMaps;
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

		virtual ~Texture();
		TextureImpl* getImpl() const;


		/**
		 * Creates a texture from an image store.
		 * The returned texture has to be released by the caller!
		 * NOTE: Supports only TEXTURE2D and CUBEMAP as targets!
		 * NOTE: Has to be implemented by renderer backend
		 *
		 * @return a TextureGL or an CubeMapGL dependent on the state of isCubeMap
		 */
		static Texture* createFromImage(const StoreImage& store, const TextureData& data, bool isCubeMap);

		// Has to be implemented by renderer backend
		static Texture* create();

		unsigned getHeight() const;
		unsigned getWidth() const;

		void setHeight(int height);
		void setWidth(int width);

	protected:

		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		Texture(TextureImpl* impl);

		int width = 0;
		int height = 0;
		TextureImpl* mImpl = nullptr;
	};

	class RenderBuffer : public Texture {
	public:

		// Has to be implemented by renderer backend
		static RenderBuffer* create();

	protected:
		// Mustn't be called by user code
		// Has to be implemented by renderer backend
		RenderBuffer();
	};


	class CubeMap : public Texture
	{
	public:

		/**
		 * Specifies the sides of a cubemap in relation to a coordinate system.
		 * E.g. in a right handed coordinate system POSITIVE_X would specifiy the right side.
		 */
		enum Side {
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
}