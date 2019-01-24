#pragma once
#include <glad/glad.h>
#include <nex/util/Math.hpp>
#include <nex/texture/Texture.hpp>

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
		RGBA32F = GL_RGBA32F,
		RGBA32I = GL_RGBA32I,
		RGBA32UI = GL_RGBA32UI,

		// srgb formats
		SRGB8 = GL_SRGB8,
		SRGBA8 = GL_SRGB8_ALPHA8,
	};

	enum PixelDataTypeGL
	{
		FLOAT = GL_FLOAT,
		UBYTE = GL_UNSIGNED_BYTE,
		UINT = GL_UNSIGNED_INT,
	};

	enum TextureTargetGl
	{
		//1D
		TEXTURE1D = GL_TEXTURE_1D,
		TEXTURE1D_ARRAY = GL_TEXTURE_1D_ARRAY,

		//2D
		TEXTURE2D = GL_TEXTURE_2D,

		// 3D
		TEXTURE2D_ARRAY = GL_TEXTURE_2D_ARRAY,
		TEXTURE3D = GL_TEXTURE_3D,

		// cubemap
		CUBE_MAP = GL_TEXTURE_CUBE_MAP,
		CUBE_POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		CUBE_NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		CUBE_POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		CUBE_NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		CUBE_POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		CUBE_NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};

	/*enum DepthStencilGL
	{
		NONE = GL_FALSE,
		DEPTH24 = GL_DEPTH_COMPONENT24,  // GL_DEPTH_COMPONENT  GL_DEPTH_COMPONENT24
		DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8, // GL_DEPTH24_STENCIL8
		DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,
	};*/

	enum DepthStencilFormatGL
	{
		DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,  // GL_DEPTH24_STENCIL8 GL_FLOAT_32_UNSIGNED_INT_24_8_REV
		DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8, //GL_DEPTH32F_STENCIL8
		DEPTH16 = GL_DEPTH_COMPONENT16,
		DEPTH24 = GL_DEPTH_COMPONENT24,
		DEPTH32 = GL_DEPTH_COMPONENT32,
		DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F, //GL_DEPTH_COMPONENT32F
		STENCIL8 = GL_STENCIL_INDEX8,  //GL_STENCIL_INDEX8
	};

	enum DepthStencilTypeGL
	{
		DEPTH = GL_DEPTH_COMPONENT,
		STENCIL = GL_STENCIL_INDEX,
		DEPTH_STENCIL = GL_DEPTH_STENCIL,
	};

	GLuint translate(bool boolean);
	TextureAccessGL translate(nex::TextureAccess);
	ChannelGL translate(nex::Channel);
	TextureFilterGL translate(nex::TextureFilter);
	TextureUVTechniqueGL translate(nex::TextureUVTechnique);
	ColorSpaceGL translate(nex::ColorSpace);
	InternFormatGL translate(nex::InternFormat);
	PixelDataTypeGL translate(nex::PixelDataType);
	TextureTargetGl translate(nex::TextureTarget);
	//DepthStencilGL translate(nex::DepthStencil);
	DepthStencilFormatGL translate(nex::DepthStencilFormat);
	DepthStencilTypeGL translate(nex::DepthStencilType);


	class TextureGL : public TextureImpl
	{
	public:
		explicit TextureGL();
		TextureGL(GLuint texture);

		virtual ~TextureGL();

		GLuint* getTexture();

		void release();

		void setTexture(GLuint id);

		static GLuint getFormat(int numberComponents);

	protected:
		friend Texture;

		GLuint mTextureID;
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

		static Side translate(CubeMap::Side side);

		explicit CubeMapGL(unsigned sideWidth, unsigned sideHeight);
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


	class DepthStencilMapGL : public Texture2DGL
	{
	public:
		explicit DepthStencilMapGL(int width, int height, const DepthStencilDesc& desc);

		virtual ~DepthStencilMapGL();

		static GLuint getDepthType(DepthStencilFormat format);
		static GLuint getDataType(DepthStencilFormat format);
		static GLuint getAttachmentType(DepthStencilFormat format);


		const DepthStencilDesc& getDescription() const;

		void resize(unsigned width, unsigned height) override;

	private:
		friend DepthStencilMap;
		DepthStencilDesc mDesc;
	};


	class RenderBufferGL : public TextureGL {
	public:
		RenderBufferGL(GLuint width, GLuint height, DepthStencilFormat format);
		virtual ~RenderBufferGL();
		RenderBufferGL(GLuint texture, GLuint width, GLuint height, DepthStencilFormat format);


		DepthStencilFormat getFormat() const;

		void resize(unsigned width, unsigned height);

	private:
		DepthStencilFormat mFormat;
		unsigned mWidth;
		unsigned mHeight;
	};
}