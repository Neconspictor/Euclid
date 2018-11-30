#pragma once
#include <nex/util/Math.hpp>

namespace nex
{
	struct StoreImage;
	class RenderTarget;
	class CubeRenderTarget;
	class BaseRenderTarget;
	class CubeMap;

	enum class TextureFilter
	{
		NearestNeighbor,
		Linear,
		Bilinear,
		Near_Mipmap_Near,     // trilinear filtering with double nearest neighbor filtering
		Near_Mipmap_Linear,   // trilinear filtering from nearest neighbor to bilinear filtering
		Linear_Mipmap_Near,   // trilinear filtering from bilinear to nearest neighbor filtering
		Linear_Mipmap_Linear, // trilinear filtering from bilinear to bilinear filtering
	};

	enum class TextureUVTechnique
	{
		ClampToBorder,
		ClampToEdge,
		MirrorRepeat,
		MirrorClampToEdge,
		Repeat,
	};

	enum class ColorSpace {
		R,
		RG,
		RGB,
		RGBA,

		// srgb formats
		SRGB,
		SRGBA,
	};

	enum class InternFormat
	{
		R8,
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
		SRGBA8,
	};

	enum class PixelDataType
	{
		FLOAT,
		UBYTE,
		UINT,
	};

	enum class TextureTarget
	{
		//1D
		TEXTURE1D,
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
		CUBE_NEGATIVE_Z,
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


	class Texture
	{
	public:
		explicit Texture();


		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		virtual ~Texture();


		/**
		 * Creates a texture from an image store.
		 * The returned texture has to be released by the caller!
		 * NOTE: Supports only TEXTURE2D and CUBEMAP as targets!
		 *
		 * @return a TextureGL or an CubeMapGL dependent on the state of isCubeMap
		 */
		static Texture* createFromImage(const StoreImage& store, const TextureData& data, bool isCubeMap);

		unsigned getHeight() const;
		unsigned getWidth() const;

		virtual void release();

		void setHeight(int height);
		void setWidth(int width);

	protected:
		int width;
		int height;
	};

	class RenderBuffer : public Texture {
	public:
		RenderBuffer();
		virtual ~RenderBuffer();

		RenderBuffer(const RenderBuffer&) = delete;
		RenderBuffer& operator=(const RenderBuffer&) = delete;

		void release() override;
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

		explicit CubeMap();

		CubeMap(const CubeMap& other) = delete;
		CubeMap& operator=(const CubeMap& other) = delete;

		virtual ~CubeMap() = default;

		/**
		 *  Generates mipmaps for the current content of this cubemap.
		 */
		void generateMipMaps();

		/**
		 * Provides a 'look at' view matrix for a specific cubemap side
		 * The returned view matrix is for right handed coordinate systems
		 */
		static const glm::mat4& getViewLookAtMatrixRH(Side side);

		friend CubeRenderTarget;

	protected:
		static glm::mat4 rightSide;
		static glm::mat4 leftSide;
		static glm::mat4 topSide;
		static glm::mat4 bottomSide;
		static glm::mat4 frontSide;
		static glm::mat4 backSide;
	};

	class BaseRenderTarget {
	public:
		explicit BaseRenderTarget(int width, int height);
		virtual ~BaseRenderTarget();


		BaseRenderTarget(const BaseRenderTarget& other) = delete;
		BaseRenderTarget& operator=(const BaseRenderTarget& other) = delete;

		void copyFrom(BaseRenderTarget* dest, const Dimension& sourceDim, int components);

		int getHeight() const
		{
			return height;
		}

		int getWidth() const
		{
			return width;
		}

	protected:
		int width, height;
	};


	class CubeRenderTarget : public BaseRenderTarget
	{
	public:
		explicit CubeRenderTarget(int width, int height, TextureData data);

		CubeRenderTarget(const CubeRenderTarget&) = delete;
		CubeRenderTarget& operator=(const CubeRenderTarget&) = delete;

		virtual ~CubeRenderTarget();



		CubeMap* createCopy();

		CubeMap* getCubeMap();

		inline int getHeightMipLevel(unsigned int mipMapLevel) const {
			return (int)(height * std::pow(0.5, mipMapLevel));
		}

		inline int getWidthMipLevel(unsigned int mipMapLevel) const {
			return (int)(height * std::pow(0.5, mipMapLevel));
		}

		void release();

		void resizeForMipMap(unsigned int mipMapLevel);

	protected:
		CubeMap* cubeMapResult;
		TextureData data;
	};


	class RenderTarget : public BaseRenderTarget
	{
	public:
		explicit RenderTarget(int width, int height);

		RenderTarget(const RenderTarget& other) = delete;
		RenderTarget& operator=(const RenderTarget& other) = delete;


		virtual ~RenderTarget();

		static RenderTarget* createMultisampled(int width, int height, const TextureData& data,
			unsigned samples, DepthStencil depthStencilType);

		static RenderTarget* createSingleSampled(int width, int height, const TextureData& data, DepthStencil depthStencilType);

		static RenderTarget* createVSM(int width, int height);

		Texture* getTexture();

		void release();

		void setTexture(Texture* texture);

	protected:
		friend CubeRenderTarget;
		Texture* textureBuffer;
	};


	class CubeDepthMap : public BaseRenderTarget
	{
	public:
		explicit CubeDepthMap(int width, int height);

		CubeDepthMap(const CubeDepthMap& other) = delete;
		CubeDepthMap& operator=(const CubeDepthMap& other) = delete;

		virtual ~CubeDepthMap();

		CubeMap* getCubeMap();

	private:
		CubeMap* cubeMap;
		glm::mat4 matrices[6];
	};


	class DepthMap : public BaseRenderTarget
	{
	public:
		explicit DepthMap(int width, int height);

		DepthMap(const DepthMap& other) = delete;
		DepthMap& operator=(const DepthMap& other) = delete;


		virtual ~DepthMap();

		Texture* getTexture();

		void release();

	private:
		Texture* texture;
	};

	class PBR_GBufferGL : public BaseRenderTarget {
	public:
		explicit PBR_GBufferGL(int width, int height);


		PBR_GBufferGL(const PBR_GBufferGL&) = delete;
		PBR_GBufferGL& operator=(const PBR_GBufferGL&) = delete;

		virtual ~PBR_GBufferGL() {}

		Texture* getAlbedo() const;
		Texture* getAoMetalRoughness() const;
		Texture* getNormal() const;
		Texture* getPosition() const;
		Texture* getDepth() const;


	protected:
		Texture* albedo;
		Texture* aoMetalRoughness;
		Texture* normal;
		Texture* position;
		RenderBuffer* depth;
	};
}