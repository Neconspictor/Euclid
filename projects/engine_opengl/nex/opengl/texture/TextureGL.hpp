#pragma once
#include <glad/glad.h>
#include <nex/util/Math.hpp>
#include <nex/texture/Texture.hpp>

struct StoreImageGL;
class RenderTargetGL;
class RendererOpenGL;
class CubeRenderTargetGL;
class BaseRenderTargetGL;
class CubeMapGL;

namespace nex
{

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
		RG = GL_RG,
		RGB = GL_RGB,
		RGBA = GL_RGBA,

		// srgb formats
		SRGB = GL_SRGB,
		SRGBA = GL_SRGB_ALPHA,
	};

	enum InternFormatGL
	{
		R8 = GL_R8,
		R16 = GL_R16,
		R32F = GL_R32F,
		R32I = GL_R32I,
		R32UI = GL_R32UI,

		RG8 = GL_RG8,
		RG16 = GL_RG16,
		RG32F = GL_RG32F,
		RG32I = GL_RG32I,
		RG32UI = GL_RG32UI,

		RGB8 = GL_RGB8,
		RGB16 = GL_RGB16,
		RGB32F = GL_RGB32F,
		RGB32I = GL_RGB32I,
		RGB32UI = GL_RGB32UI,

		RGBA8 = GL_RGBA8,
		RGBA16 = GL_RGBA16,
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
		CUBE_POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		CUBE_NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		CUBE_POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		CUBE_NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		CUBE_POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		CUBE_NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};


	bool isCubeTarget(TextureTarget target);

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
		TextureGL(GLuint texture);

		virtual ~TextureGL();


		/**
		 * Creates a texture from an image store.
		 * The returned texture has to be released by the caller!
		 * NOTE: Supports only TEXTURE2D and CUBEMAP as targets!
		 *
		 * @return a TextureGL or an CubeMapGL dependent on the state of isCubeMap
		 */
		static TextureGL* createFromImage(const StoreImageGL& store, const TextureData& data, bool isCubeMap);

		GLuint getTexture() const;


		unsigned getHeight() const;
		unsigned getWidth() const;

		void setTexture(GLuint id);

		static GLuint getFormat(int numberComponents);

		void setHeight(int height);
		void setWidth(int width);

	protected:
		GLuint mTextureID;
		int width;
		int height;
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

		explicit CubeMapGL();
		CubeMapGL(GLuint cubeMap);

		/**
		 *  Generates mipmaps for the current content of this cubemap.
		 */
		void generateMipMaps();

		GLuint getCubeMap() const;

		/**
		 * Provides a 'look at' view matrix for a specific cubemap side
		 * The returned view matrix is for right handed coordinate systems
		 */
		static const glm::mat4& getViewLookAtMatrixRH(Side side);

		void setCubeMap(GLuint id);

		friend CubeRenderTargetGL;

	protected:
		static glm::mat4 rightSide;
		static glm::mat4 leftSide;
		static glm::mat4 topSide;
		static glm::mat4 bottomSide;
		static glm::mat4 frontSide;
		static glm::mat4 backSide;
	};


	class RenderBufferGL : public TextureGL {
	public:
		RenderBufferGL();
		virtual ~RenderBufferGL();
		RenderBufferGL(GLuint texture);
	};
}