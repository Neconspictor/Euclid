#pragma once

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
		RED_INTEGER,
		RG,
		RG_INTEGER,
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

	enum class CompareFunction
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

	struct SamplerDesc
	{
		glm::vec4 borderColor = { 0,0,0,0 };
		TextureFilter minFilter = TextureFilter::Linear;
		TextureFilter magFilter = TextureFilter::Linear;
		TextureUVTechnique wrapS = TextureUVTechnique::ClampToEdge;
		TextureUVTechnique wrapT = TextureUVTechnique::ClampToEdge;
		TextureUVTechnique wrapR = TextureUVTechnique::ClampToEdge;
		int minLOD = -1000;
		int maxLOD = 1000;
		float biasLOD = 0;
		bool useDepthComparison = false; // Only used for depth-stencil maps
		CompareFunction compareFunction = CompareFunction::LESS_EQUAL;
		float maxAnisotropy = 1.0f;

		static SamplerDesc createMipMapRepeat()
		{
			SamplerDesc desc;
			desc.minFilter = TextureFilter::Linear_Mipmap_Linear;
			desc.wrapS = desc.wrapR = desc.wrapT = TextureUVTechnique::Repeat;
			return desc;
		}
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
		ColorSpace colorspace = ColorSpace::RGBA;
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

		static TextureData createRenderTargetRGBAHDR()
		{
			TextureData data;
			data.generateMipMaps = false;
			data.minFilter = TextureFilter::Linear;
			data.magFilter = TextureFilter::Linear;
			data.colorspace = ColorSpace::RGBA;
			data.internalFormat = InternFormat::RGBA32F;
			data.pixelDataType = PixelDataType::FLOAT;
			data.wrapR = TextureUVTechnique::ClampToEdge;
			data.wrapS = TextureUVTechnique::ClampToEdge;
			data.wrapT = TextureUVTechnique::ClampToEdge;

			return data;
		}
	};
}