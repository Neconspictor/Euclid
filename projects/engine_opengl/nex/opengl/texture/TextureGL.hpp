#pragma once
#include <nex/texture/Texture.hpp>
#include <nex/opengl/opengl.hpp>

namespace nex
{
	struct StoreImage;

	enum TextureAccessGL
	{
		READ_ONLY = GL_READ_ONLY,
		READ_WRITE = GL_READ_WRITE,
		WRITE_ONLY = GL_WRITE_ONLY,
	};

	enum ChannelGL
	{
		RED = GL_RED,
		GREEN = GL_GREEN,
		BLUE = GL_BLUE,
		ALPHA = GL_ALPHA,
	};

	enum TextureFilterGL
	{
		NearestNeighbor = GL_NEAREST,
		Linear = GL_LINEAR,
		Near_Mipmap_Near = GL_NEAREST_MIPMAP_NEAREST,     // trilinear filtering with double nearest neighbor filtering
		Near_Mipmap_Linear = GL_NEAREST_MIPMAP_LINEAR,   // trilinear filtering from nearest neighbor to bilinear filtering
		Linear_Mipmap_Near = GL_LINEAR_MIPMAP_NEAREST,   // trilinear filtering from bilinear to nearest neighbor filtering
		Linear_Mipmap_Linear = GL_LINEAR_MIPMAP_LINEAR, // trilinear filtering from bilinear to bilinear filtering
	};

	enum TextureUVTechniqueGL
	{
		ClampToBorder = GL_CLAMP_TO_BORDER,
		ClampToEdge = GL_CLAMP_TO_EDGE,
		MirrorRepeat = GL_MIRRORED_REPEAT,
		MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE,
		Repeat = GL_REPEAT,
	};

	enum ColorSpaceGL {
		R = GL_RED,
		RED_INTEGER = GL_RED_INTEGER,
		RG = GL_RG,
		RGB = GL_RGB,
		RGBA = GL_RGBA,

		// srgb formats
		SRGB = RGB,
		SRGBA = RGBA,

		DEPTH = GL_DEPTH_COMPONENT,
		STENCIL = GL_STENCIL_COMPONENTS,
		DEPTH_STENCIL = GL_DEPTH_STENCIL,
	};

	enum InternFormatGL
	{
		R8 = GL_R8,
		R16 = GL_R16,
		R16F = GL_R16F,
		R32F = GL_R32F,
		R32I = GL_R32I,
		R32UI = GL_R32UI,

		RG8 = GL_RG8,
		RG16 = GL_RG16,
		RG16F = GL_RG16F,
		RG32F = GL_RG32F,
		RG32I = GL_RG32I,
		RG32UI = GL_RG32UI,

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

	enum PixelDataTypeGL
	{
		FLOAT = GL_FLOAT,
		UBYTE = GL_UNSIGNED_BYTE,
		UINT = GL_UNSIGNED_INT,
		SHORT = GL_SHORT,

		FLOAT_32_UNSIGNED_INT_24_8_REV = GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
		UNSIGNED_INT_24_8 = GL_UNSIGNED_INT_24_8,
		UNSIGNED_INT_8 = GL_UNSIGNED_BYTE,
		UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
		UNSIGNED_INT_24 = GL_UNSIGNED_INT,
	};

	enum TextureTargetGl
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
	TextureFilterGL translate(nex::TextureFilter);
	TextureUVTechniqueGL translate(nex::TextureUVTechnique);
	ColorSpaceGL translate(nex::ColorSpace);
	InternFormatGL translate(nex::InternFormat);
	PixelDataTypeGL translate(nex::PixelDataType);
	TextureTargetGl translate(nex::TextureTarget);



	class TextureGL : public TextureImpl
	{
	public:
		explicit TextureGL();
		TextureGL(GLuint texture, GLuint target);

		virtual ~TextureGL();

		static std::unique_ptr<TextureGL> createView(TextureGL* original,
			TextureTarget target,
			unsigned minLevel,
			unsigned numLevel,
			unsigned minLayer,
			unsigned numLayers,
			const TextureData& data);

		static void generateTexture(GLuint* out, const BaseTextureDesc& desc, GLenum target);

		static void applyTextureData(GLuint texture, const BaseTextureDesc& desc, GLenum target);

		static GLuint getFormat(int numberComponents);

		GLuint* getTexture();

		void readback(TextureTarget target, unsigned mipmapLevel, ColorSpace format, PixelDataType type, void* dest, CubeMapSide side = CubeMapSide::POSITIVE_X);

		void release();

		static void resizeTexImage2D(
			GLuint textureID,
			GLenum target,
			GLint level,
			unsigned width,
			unsigned height,
			GLenum  colorspace,
			GLint  internalFormat,
			GLenum  pixelDataType,
			bool generateMipMaps,
			const void* data);


		static void resizeTexImage3D(
			GLuint textureID,
			GLenum target,
			GLint level,
			unsigned width,
			unsigned height,
			unsigned depth,
			GLenum  colorspace,
			GLint  internalFormat,
			GLenum  pixelDataType,
			bool generateMipMaps,
			const void* data);

		void setTexture(GLuint id);
		GLuint getTarget() const;

	protected:
		friend Texture;

		GLuint mTextureID;
		GLuint mTarget;
	};

	class Texture2DGL : public TextureGL
	{
	public:
		explicit Texture2DGL(GLuint width, GLuint height, const TextureData& textureData, const void* data);
		Texture2DGL(GLuint texture, const TextureData& textureData, unsigned width = 0, unsigned height = 0);

		virtual ~Texture2DGL() = default;

		unsigned getWidth() const;
		unsigned getHeight() const;

		void setHeight(int height);
		void setWidth(int width);

		virtual void resize(unsigned width, unsigned height);

	protected:
		friend Texture2D;
		unsigned mWidth;
		unsigned mHeight;
		unsigned mSamples;
		TextureData mData;
	};

	class Texture2DMultisampleGL : public Texture2DGL
	{
	public:
		Texture2DMultisampleGL(GLuint width, GLuint height, const TextureData& textureData, unsigned samples = 1);
		Texture2DMultisampleGL(GLuint texture, const TextureData& textureData, unsigned samples = 1, unsigned width = 0, unsigned height = 0);

		virtual ~Texture2DMultisampleGL() = default;

		void resize(unsigned width, unsigned height) override;
		unsigned getSamples() const;

	protected:
		unsigned mSamples;
	};

	class Texture2DArrayGL : public TextureGL
	{
	public:
		explicit Texture2DArrayGL(GLuint width, GLuint height, GLuint size, const TextureData& textureData, const void* data);
		Texture2DArrayGL(GLuint texture, const TextureData& textureData, unsigned width = 0, unsigned height = 0, unsigned size = 0);

		virtual ~Texture2DArrayGL() = default;

		unsigned getWidth() const;
		unsigned getHeight() const;
		unsigned getSize() const;

		void setHeight(unsigned height);
		void setWidth(unsigned width);
		void setSize(unsigned size);

		void resize(unsigned width, unsigned height, unsigned size);

	protected:
		friend Texture2DArray;
		unsigned mWidth;
		unsigned mHeight;
		unsigned mSize;
		TextureData mData;
	};

	class CubeMapGL : public TextureGL
	{
	public:

		/**
		 * Specifies the sides of a cubemap in relation to a coordinate system.
		 * E.g. in a right handed coordinate system POSITIVE_X would specifiy the right side.
		 */
		enum Side {
			POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};

		static Side translate(CubeMapSide side);

		explicit CubeMapGL(unsigned sideWidth, unsigned sideHeight, const TextureData& data);
		CubeMapGL(GLuint cubeMap, unsigned sideWidth, unsigned sideHeight);

		/**
		 *  Generates mipmaps for the current content of this cubemap.
		 */
		void generateMipMaps();

		GLuint getCubeMap() const;

		unsigned getSideWidth() const;
		unsigned getSideHeight() const;

		void setCubeMap(GLuint id);

		void setSideWidth(unsigned width);
		void setSideHeight(unsigned height);

	protected:
		unsigned mSideWidth;
		unsigned mSideHeight;
	};


	class RenderBufferGL : public TextureGL {
	public:
		RenderBufferGL(GLuint width, GLuint height, InternFormat format);
		virtual ~RenderBufferGL();
		RenderBufferGL(GLuint texture, GLuint width, GLuint height, InternFormat format);


		InternFormat getFormat() const;

		void resize(unsigned width, unsigned height);

	private:
		InternFormat mFormat;
		unsigned mWidth;
		unsigned mHeight;
	};
}