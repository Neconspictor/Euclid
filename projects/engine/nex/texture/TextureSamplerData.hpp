#pragma once
#include "nex/math/Constant.hpp"
#include <glm/glm.hpp>

namespace nex
{
	enum class TextureAccess
	{
		READ_ONLY, FIRST = READ_ONLY,
		READ_WRITE,
		WRITE_ONLY, LAST = WRITE_ONLY,
	};

	enum class ColorSpace {
		R, FIRST = R,
		RED_INTEGER, // bot unsigned and signed formats
		
		RG,
		RG_INTEGER, // bot unsigned and signed formats

		RGB,
		BGR,
		RGB_INTEGER, // bot unsigned and signed formats
		BGR_INTEGER,

		RGBA,
		BGRA,
		RGBA_INTEGER, // bot unsigned and signed formats
		BGRA_INTEGER,
		
		DEPTH,
		STENCIL,
		DEPTH_STENCIL, 
		
		LAST = DEPTH_STENCIL,
	};

	/**
	 * Specifies which component should be sampled from a depth-stencil texture in shader code.
	 */
	enum class DepthStencilTexMode {
		DEPTH, FIRST = DEPTH,
		STENCIL, LAST = STENCIL,
	};

	enum class InternalFormatType {
		FLOAT,
		INT,
		UINT,
		SNORM,
		NORMAL,
		COMBINED,
	};

	enum class InternalFormat
	{
		R8, FIRST = R8, //Note: nothing specified after the bit depth means UNORM = unsigned normalized integers (range [0, 1]) , see: https://www.khronos.org/opengl/wiki/Normalized_Integer
		R8UI,
		R16,
		R16F,
		R32F,
		R32I,
		R32UI,

		RG8,
		RG8UI,
		RG8_SNORM, // SNORM = signed normalized integers (range [-1, 1]) , see: https://www.khronos.org/opengl/wiki/Normalized_Integer
		RG16,
		RG16F,
		RG32F,
		RG32I,
		RG32UI,

		RGB5,
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

		RGB10_A2,
		RGB10_A2UI,

		// srgb formats
		SRGB8,
		SRGBA8,

		DEPTH24_STENCIL8,
		DEPTH32F_STENCIL8, 
		DEPTH16,
		DEPTH24,
		DEPTH32,
		DEPTH_COMPONENT32F,
		STENCIL8, LAST = STENCIL8,
	};

	enum class PixelDataType
	{
		FLOAT, FIRST = FLOAT, // Each component is a 4 byte float
		FLOAT_HALF, // Each coponent is a 2 byte float
		INT, // Each component is an int
		UBYTE, // each component is an unsigned char
		UINT, // each comoponent is an unsigned int

		SHORT, // each copmonent is a short

		UNSIGNED_INT_24_8, // two components are packed into an unsigned int; the first component uses 24 bits, the second 8 bits; Useful for depth-stencil textures
		FLOAT_32_UNSIGNED_INT_24_8_REV, // two components are packed to a group of 64 bits; The first component is a 32bit float; the second component is 8bit integer; the last 24 bits are unused;
		UNSIGNED_SHORT, // Each component is an unsigned short
		UNSIGNED_INT_24, //Each component is assigned to an unsigned int of 24 bits; the last 8 bits are unused;
		UNSIGNED_INT_8_8_8_8,  // Four components are grouped into an unsigned int; each component has 8 bits;
		
		UNSIGNED_INT_10_10_10_2, // Four components are grouped into an unsigned int; 3 compontents have 10 bits; the fourth copmonent has 2 bits;
		
		LAST = UNSIGNED_INT_10_10_10_2,
	};

	enum class Channel
	{
		RED, FIRST = RED,
		GREEN,
		BLUE,
		ALPHA,
		ONE,
		ZERO,
		LAST = ZERO,
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
		CUBE_MAP, 
		CUBE_MAP_ARRAY,
		
		RENDER_BUFFER,
		
		LAST = RENDER_BUFFER,
	};

		/**
		 * Specifies the sides of a cubemap in relation to a coordinate system.
		 * E.g. in a right handed coordinate system POSITIVE_X would specifiy the right side.
		 */
	enum class CubeMapSide {
		POSITIVE_X = 0,
		NEGATIVE_X,
		POSITIVE_Y,
		NEGATIVE_Y,
		POSITIVE_Z,
		NEGATIVE_Z,
	};

	enum class CompFunc
	{
		ALWAYS, FIRST = ALWAYS,
		EQUAL,
		GREATER,
		GREATER_EQUAL,
		LESS,
		LESS_EQUAL,
		NEVER,
		NOT_EQUAL, LAST = NOT_EQUAL
	};

	enum class TexFilter
	{
		Nearest, FIRST = Nearest,
		Linear,
		Near_Mipmap_Near,     // trilinear filtering with double nearest neighbor filtering
		Near_Mipmap_Linear,   // trilinear filtering from nearest neighbor to bilinear filtering
		Linear_Mipmap_Near,   // trilinear filtering from bilinear to nearest neighbor filtering
		Linear_Mipmap_Linear, LAST = Linear_Mipmap_Linear,// trilinear filtering from bilinear to bilinear filtering
	};

	enum class UVTechnique
	{
		ClampToBorder, FIRST = ClampToBorder,
		ClampToEdge,
		MirrorRepeat,
		MirrorClampToEdge,
		Repeat, LAST = Repeat,
	};

	struct SamplerDesc
	{
		glm::vec4 borderColor = { 0,0,0,0 };
		TexFilter minFilter = TexFilter::Linear;
		TexFilter magFilter = TexFilter::Linear;
		UVTechnique wrapS = UVTechnique::ClampToEdge;
		UVTechnique wrapT = UVTechnique::ClampToEdge;
		UVTechnique wrapR = UVTechnique::ClampToEdge;
		int minLOD = -1000;
		int maxLOD = 1000;
		float biasLOD = 0;
		bool useDepthComparison = false; // Only used for depth-stencil maps
		CompFunc compareFunction = CompFunc::LESS;
		float maxAnisotropy = 1.0f;

		static SamplerDesc createMipMapRepeat()
		{
			SamplerDesc desc;
			desc.minFilter = TexFilter::Linear_Mipmap_Linear;
			desc.wrapS = desc.wrapR = desc.wrapT = UVTechnique::Repeat;
			return desc;
		}
	};

	struct BaseTextureDesc : public SamplerDesc
	{
		bool generateMipMaps = false;
		unsigned lodBaseLevel = 0; // index of the lowest defined mipmap level
		unsigned lodMaxLevel = 1000.0f; //index of the highest defined mipmap level
		glm::vec<4, Channel, glm::highp> swizzle = { Channel::RED, Channel::GREEN, Channel::BLUE, Channel::ALPHA };
		unsigned textureIndex = 0;
		bool useSwizzle = false;

		/**
		 * Only used for depth-stencil textures
		 */
		DepthStencilTexMode depthStencilTextureMode = DepthStencilTexMode::DEPTH;
	};


	struct TextureDesc : public BaseTextureDesc
	{
		InternalFormat internalFormat = InternalFormat::RGBA8;

		TextureDesc() {}

		TextureDesc(TexFilter minFilter, 
			TexFilter magFilter, 
			UVTechnique wrapR,
			UVTechnique wrapS,
			UVTechnique wrapT,
			InternalFormat internalFormat,
			bool generateMipMaps) : 
			internalFormat(internalFormat)
		{
			this->minFilter = minFilter;
			this->magFilter = magFilter;
			this->wrapR = wrapR;
			this->wrapS = wrapS;
			this->wrapT = wrapT;
			this->generateMipMaps = generateMipMaps;
		}

		static TextureDesc createImage(TexFilter minFilter,
			TexFilter magFilter,
			UVTechnique wrapR,
			UVTechnique wrapS,
			UVTechnique wrapT,
			InternalFormat internalFormat,
			bool generateMipMaps)
		{
			TextureDesc result;
			result.internalFormat = internalFormat;
			result.minFilter = minFilter;
			result.magFilter = magFilter;
			result.wrapS = wrapR;
			result.wrapR = wrapS;
			result.wrapT = wrapT;
			result.generateMipMaps = generateMipMaps;
			return result;
		}

		static TextureDesc createDepth(CompFunc compareFunction, 
			InternalFormat format)
		{
			TextureDesc result;
			result.useDepthComparison = true;
			result.compareFunction = compareFunction;
			result.internalFormat = format;
			result.minFilter = result.magFilter = TexFilter::Nearest;
			result.wrapS = result.wrapR = result.wrapT = UVTechnique::ClampToEdge;
			return result;
		}

		static TextureDesc createRenderTargetRGBAHDR(InternalFormat format = InternalFormat::RGBA32F, bool generateMipMaps = false)
		{
			TextureDesc data;
			data.generateMipMaps = generateMipMaps;
			data.minFilter = TexFilter::Nearest;
			data.magFilter = TexFilter::Nearest;
			data.internalFormat = format;
			data.wrapR = UVTechnique::ClampToEdge;
			data.wrapS = UVTechnique::ClampToEdge;
			data.wrapT = UVTechnique::ClampToEdge;

			return data;
		}
	};


	ColorSpace getColorSpace(InternalFormat format);
	unsigned  getComponents(InternalFormat format);
	unsigned getComponents(const ColorSpace colorspace);
	unsigned getPixelDataTypeByteSize(const PixelDataType pixelDataType);
	unsigned getPixelDataTypePackedComponentsCount(const PixelDataType pixelDataType);
	InternalFormatType getType(InternalFormat format);
}