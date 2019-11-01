#pragma once
#include <nex/texture/Texture.hpp>
#include <nex/opengl/opengl.hpp>

namespace nex
{
	struct StoreImage;

	enum class TextureAccessGL
	{
		READ_ONLY = GL_READ_ONLY,
		READ_WRITE = GL_READ_WRITE,
		WRITE_ONLY = GL_WRITE_ONLY,
	};

	enum class ChannelGL
	{
		RED = GL_RED,
		GREEN = GL_GREEN,
		BLUE = GL_BLUE,
		ALPHA = GL_ALPHA,
		ONE = GL_ONE,
		ZERO = GL_ZERO,
	};

	enum class TextureFilterGL
	{
		NearestNeighbor = GL_NEAREST,
		Linear = GL_LINEAR,
		Near_Mipmap_Near = GL_NEAREST_MIPMAP_NEAREST,     // trilinear filtering with double nearest neighbor filtering
		Near_Mipmap_Linear = GL_NEAREST_MIPMAP_LINEAR,   // trilinear filtering from nearest neighbor to bilinear filtering
		Linear_Mipmap_Near = GL_LINEAR_MIPMAP_NEAREST,   // trilinear filtering from bilinear to nearest neighbor filtering
		Linear_Mipmap_Linear = GL_LINEAR_MIPMAP_LINEAR, // trilinear filtering from bilinear to bilinear filtering
	};

	enum class TextureUVTechniqueGL
	{
		ClampToBorder = GL_CLAMP_TO_BORDER,
		ClampToEdge = GL_CLAMP_TO_EDGE,
		MirrorRepeat = GL_MIRRORED_REPEAT,
		MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE,
		Repeat = GL_REPEAT,
	};

	enum class ColorSpaceGL {
		R = GL_RED,
		RED_INTEGER = GL_RED_INTEGER,
		RG = GL_RG,
		RG_INTEGER = GL_RG_INTEGER,
		RGB = GL_RGB,
		RGBA = GL_RGBA,

		// srgb formats
		SRGB = RGB,
		SRGBA = RGBA,

		DEPTH = GL_DEPTH_COMPONENT,
		STENCIL = GL_STENCIL_INDEX,
		DEPTH_STENCIL = GL_DEPTH_STENCIL,
	};

	enum class DepthStencilTextureModeGL {
		DEPTH = GL_DEPTH_COMPONENT,
		STENCIL = GL_STENCIL_INDEX,
	};

	enum class InternFormatGL
	{
		R8 = GL_R8,
		R8UI = GL_R8UI,
		R16 = GL_R16,
		R16F = GL_R16F,
		R32F = GL_R32F,
		R32I = GL_R32I,
		R32UI = GL_R32UI,

		RG8 = GL_RG8,
		RG8UI = GL_RG8UI,
		RG8_SNORM = GL_RG8_SNORM,
		RG16 = GL_RG16,
		RG16F = GL_RG16F,
		RG32F = GL_RG32F,
		RG32I = GL_RG32I,
		RG32UI = GL_RG32UI,

		RGB5 = GL_RGB5,
		RGB8 = GL_RGB8,
		RGB16 = GL_RGB16,
		RGB16F = GL_RGB16F,
		RGB32F = GL_RGB32F,
		RGB32I = GL_RGB32I,
		RGB32UI = GL_RGB32UI,

		RGBA8 = GL_RGBA8,
		RGBA16 = GL_RGBA16,
		RGBA16F = GL_RGBA16F,
		RGBA16_SNORM = GL_RGBA16_SNORM,
		RGBA32F = GL_RGBA32F,
		RGBA32I = GL_RGBA32I,
		RGBA32UI = GL_RGBA32UI,

		RGB10_A2 = GL_RGB10_A2,
		RGB10_A2UI = GL_RGB10_A2UI,


		// srgb formats
		SRGB8 = GL_SRGB8,
		SRGBA8 = GL_SRGB8_ALPHA8,

		DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,  // GL_DEPTH24_STENCIL8 GL_FLOAT_32_UNSIGNED_INT_24_8_REV
		DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8, //GL_DEPTH32F_STENCIL8
		DEPTH16 = GL_DEPTH_COMPONENT16,
		DEPTH24 = GL_DEPTH_COMPONENT24,
		DEPTH32 = GL_DEPTH_COMPONENT32,
		DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F, //GL_DEPTH_COMPONENT32F
		STENCIL8 = GL_STENCIL_INDEX8,   //GL_STENCIL_INDEX8
	};

	enum class PixelDataTypeGL
	{
		FLOAT = GL_FLOAT,
		FLOAT_HALF = GL_HALF_FLOAT,
		INT = GL_INT,
		UBYTE = GL_UNSIGNED_BYTE,
		UINT = GL_UNSIGNED_INT,
		SHORT = GL_SHORT,

		FLOAT_32_UNSIGNED_INT_24_8_REV = GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
		UNSIGNED_INT_24_8 = GL_UNSIGNED_INT_24_8,
		UNSIGNED_INT_8 = GL_UNSIGNED_BYTE,
		UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
		UNSIGNED_INT_24 = GL_UNSIGNED_INT,

		UNSIGNED_INT_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2,
	};

	enum class TextureTargetGl
	{
		//1D
		TEXTURE1D = GL_TEXTURE_1D,
		TEXTURE1D_ARRAY = GL_TEXTURE_1D_ARRAY,

		//2D
		TEXTURE2D = GL_TEXTURE_2D,
		TEXTURE2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,

		// 3D
		TEXTURE2D_ARRAY = GL_TEXTURE_2D_ARRAY,
		TEXTURE2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
		TEXTURE3D = GL_TEXTURE_3D,

		// cubemap
		CUBE_MAP = GL_TEXTURE_CUBE_MAP,
		CUBE_MAP_ARRAY = GL_TEXTURE_CUBE_MAP_ARRAY,

		RENDERBUFFER = GL_RENDERBUFFER,
	};

	/*enum DepthStencilGL
	{
		NONE = GL_FALSE,
		DEPTH24 = GL_DEPTH_COMPONENT24,  // GL_DEPTH_COMPONENT  GL_DEPTH_COMPONENT24
		DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8, // GL_DEPTH24_STENCIL8
		DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,
	};*/

	TextureAccessGL translate(nex::TextureAccess);
	ChannelGL translate(nex::Channel);
	TextureFilterGL translate(nex::TexFilter);
	TextureUVTechniqueGL translate(nex::UVTechnique);
	ColorSpaceGL translate(nex::ColorSpace);
	DepthStencilTextureModeGL translate(nex::DepthStencilTexMode);
	InternFormatGL translate(nex::InternalFormat);
	PixelDataTypeGL translate(nex::PixelDataType);
	TextureTargetGl translate(nex::TextureTarget);



	class Texture::Impl
	{
	public:
		explicit Impl(TextureTarget target, const TextureDesc& data, unsigned width, unsigned height, unsigned depth);
		Impl(GLuint texture, TextureTarget target, const TextureDesc& data, unsigned width, unsigned height, unsigned depth);

		virtual ~Impl();
		const TextureDesc& getTextureData() const;

		static void applyTextureData(GLuint texture, const BaseTextureDesc& desc);

		static std::unique_ptr<Impl> createView(Impl* original,
			TextureTarget target,
			unsigned minLevel,
			unsigned numLevel,
			unsigned minLayer,
			unsigned numLayers,
			const TextureDesc& data);

		void generateMipMaps();

		static void generateTexture(GLuint* out, const BaseTextureDesc& desc, GLenum target);


		static GLuint getFormat(int numberComponents);

		GLuint* getTexture();

		void release();

		static void resizeTexImage1D(
			GLuint textureID,
			GLint levels,
			unsigned width,
			GLint  internalFormat,
			bool generateMipMaps);

		static void resizeTexImage2D(
			GLuint textureID,
			GLint levels,
			unsigned width,
			unsigned height,
			GLint  internalFormat,
			bool generateMipMaps);


		static void resizeTexImage3D(
			GLuint textureID,
			GLint levels,
			unsigned width,
			unsigned height,
			unsigned depth,
			GLint  internalFormat,
			bool generateMipMaps);

		void setTexture(GLuint id);
		TextureTarget getTarget() const;
		TextureTargetGl getTargetGL() const;

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

		void setHeight(unsigned height);
		void setWidth(unsigned width);
		void setDepth(unsigned depth);

		void updateMipMapCount();

	protected:
		friend Texture;

		GLuint mTextureID;
		TextureTarget mTarget;
		TextureTargetGl mTargetGL;
		TextureDesc mTextureData;

		unsigned mWidth;
		unsigned mHeight;
		unsigned mDepth;
	};

	class Texture1DGL : public Texture::Impl
	{
	public:
		explicit Texture1DGL(GLuint width, const TextureDesc& textureData, const void* data);
		Texture1DGL(GLuint texture, const TextureDesc& textureData, unsigned width = 0);

		virtual void resize(unsigned width, unsigned mipmapCount, bool autoMipMapCount);
	};



	class Texture2DGL : public Texture::Impl
	{
	public:
		explicit Texture2DGL(GLuint width, GLuint height, const TextureDesc& textureData, const void* data);
		Texture2DGL(GLuint texture, const TextureDesc& textureData, unsigned width = 0, unsigned height = 0);

		virtual void resize(unsigned width, unsigned height, unsigned mipmapCount, bool autoMipMapCount);

	protected:
		friend Texture2D;
		unsigned mSamples;
	};

	class Texture2DMultisampleGL : public Texture2DGL
	{
	public:
		Texture2DMultisampleGL(GLuint width, GLuint height, const TextureDesc& textureData, unsigned samples = 1);
		Texture2DMultisampleGL(GLuint texture, const TextureDesc& textureData, unsigned samples = 1, unsigned width = 0, unsigned height = 0);

		/**
		 * Note: mipmapCount and autoMipMapCount aren't used for multisample textures!
		 */
		void resize(unsigned width, unsigned height, unsigned mipmapCount = 0, bool autoMipMapCount = false) override;
		unsigned getSamples() const;

	protected:
		unsigned mSamples;
	};

	class Texture2DArrayGL : public Texture::Impl
	{
	public:
		explicit Texture2DArrayGL(GLuint width, GLuint height, GLuint depth, const TextureDesc& textureData, const void* data);
		Texture2DArrayGL(GLuint texture, const TextureDesc& textureData, unsigned width = 0, unsigned height = 0, unsigned depth = 0);

		void resize(unsigned width, unsigned height, unsigned depth, unsigned mipmapCount, bool autoMipMapCount);

	protected:
		friend Texture2DArray;
	};

	class Texture3DGL : public Texture::Impl
	{
	public:
		explicit Texture3DGL(GLuint width, GLuint height, GLuint depth, const TextureDesc& textureData, const void* data);
		Texture3DGL(GLuint texture, const TextureDesc& textureData, unsigned width = 0, unsigned height = 0, unsigned depth = 0);

		void resize(unsigned width, unsigned height, unsigned depth, unsigned mipmapCount, bool autoMipMapCount);

	protected:
		friend Texture3D;
	};

	class CubeMapGL : public Texture::Impl
	{
	public:

		/**
		 * Specifies the sides of a cubemap in relation to a coordinate system.
		 * E.g. in a right handed coordinate system POSITIVE_X would specifiy the right side.
		 */
		enum class Side {
			POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};

		static Side translate(CubeMapSide side);

		explicit CubeMapGL(unsigned sideWidth, unsigned sideHeight, const TextureDesc& data);
		CubeMapGL(GLuint cubeMap, unsigned sideWidth, unsigned sideHeight, const TextureDesc& data);

		GLuint getCubeMap() const;

		void setCubeMap(GLuint id);
	};

	class CubeMapArrayGL : public Texture::Impl
	{
	public:
		explicit CubeMapArrayGL(GLuint sideWidth, GLuint sideHeight, GLuint depth, const TextureDesc& textureData, const void* data);
		CubeMapArrayGL(GLuint texture, const TextureDesc& textureData, unsigned sideWidth = 0, unsigned sideHeight = 0, unsigned depth = 0);

		void fill(unsigned xOffset, unsigned yOffset, unsigned zOffset,
			unsigned sideWidth, unsigned sideHeight, unsigned layerFaces,
			unsigned mipmapIndex,
			const void* data);

		unsigned getLayerFaces() const;

		void resize(unsigned sideWidth, unsigned sideHeight, unsigned depth, unsigned mipmapCount, bool autoMipMapCount);

	protected:
		friend Texture2DArray;
		unsigned mLayerFaces;
	};


	class RenderBufferGL : public Texture::Impl {
	public:
		RenderBufferGL(GLuint width, GLuint height, const TextureDesc& data);
		virtual ~RenderBufferGL();
		RenderBufferGL(GLuint texture, GLuint width, GLuint height, const TextureDesc& data);


		InternalFormat getFormat() const;

		void resize(unsigned width, unsigned height);
	};
}