#include <nex/opengl/texture/TextureGL.hpp>
#include <memory>
#include <cassert>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

mat4 nex::CubeMapGL::rightSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMapGL::leftSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMapGL::topSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
mat4 nex::CubeMapGL::bottomSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
mat4 nex::CubeMapGL::frontSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMapGL::backSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f));



const mat4& nex::CubeMapGL::getViewLookAtMatrixRH(Side side)
{
	switch (side) {
	case POSITIVE_X:
		return rightSide;
	case NEGATIVE_X:
		return leftSide;
	case POSITIVE_Y:
		return topSide;
	case NEGATIVE_Y:
		return bottomSide;
	case NEGATIVE_Z:
		return frontSide;
	case POSITIVE_Z:
		return backSide;
	default:
		throw_with_trace(std::runtime_error("No mapping defined for " + side));
	}

	// won't be reached
	return rightSide;
}

nex::CubeMapGL::CubeMapGL() : TextureGL() {}

nex::CubeMapGL::CubeMapGL(GLuint cubeMap) : TextureGL(cubeMap){}

void nex::CubeMapGL::generateMipMaps()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


GLuint nex::CubeMapGL::getCubeMap() const
{
	return mTextureID;
}

void nex::CubeMapGL::setCubeMap(GLuint id)
{
	mTextureID = id;
}

nex::TextureFilterGL nex::translate(nex::TextureFilter filter)
{
	static TextureFilterGL const table[]
	{
		NearestNeighbor,
		Linear,
		Near_Mipmap_Near,
		Near_Mipmap_Linear,
		Linear_Mipmap_Near,
		Linear_Mipmap_Linear,
	};

	static const unsigned size = (unsigned)TextureFilter::LAST - (unsigned)TextureFilter::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)filter];
}

nex::TextureUVTechniqueGL nex::translate(nex::TextureUVTechnique technique)
{
	static TextureUVTechniqueGL const table[]
	{
		ClampToBorder,
		ClampToEdge,
		MirrorRepeat,
		MirrorClampToEdge,
		Repeat,
	};

	static const unsigned size = (unsigned)TextureUVTechnique::LAST - (unsigned)TextureUVTechnique::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)technique];
}

nex::ColorSpaceGL nex::translate(nex::ColorSpace colorSpace)
{
	static ColorSpaceGL const table[]
	{
		R,
		RG,
		RGB,
		RGBA,

		// srgb formats
		SRGB,
		SRGBA,
	};

	static const unsigned size = (unsigned)ColorSpace::LAST - (unsigned)ColorSpace::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)colorSpace];
}

nex::InternFormatGL nex::translate(nex::InternFormat format)
{
	static InternFormatGL const table[]
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

	static const unsigned size = (unsigned)InternFormat::LAST - (unsigned)InternFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)format];
}

nex::PixelDataTypeGL nex::translate(nex::PixelDataType dataType)
{
	static PixelDataTypeGL const table[]
	{
		FLOAT,
		UBYTE,
		UINT,
	};

	static const unsigned size = (unsigned)PixelDataType::LAST - (unsigned)PixelDataType::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)dataType];
}

nex::TextureTargetGl nex::translate(nex::TextureTarget target)
{
	static TextureTargetGl const table[]
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

	static const unsigned size = (unsigned)TextureTarget::LAST - (unsigned)TextureTarget::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)target];
}

nex::TextureGL::TextureGL(): mTextureID(GL_FALSE), width(0), height(0)
{
}

nex::TextureGL::TextureGL(GLuint texture) : mTextureID(texture), width(0), height(0)
{
}


nex::TextureGL::~TextureGL()
{
	if (mTextureID != GL_FALSE) {
		GLCall(glDeleteTextures(1, &mTextureID));
		mTextureID = GL_FALSE;
	}
}

nex::TextureGL* nex::TextureGL::createFromImage(const StoreImageGL& store, const nex::TextureData& data, bool isCubeMap)
{
	GLuint format = static_cast<GLuint>(data.colorspace);
	GLuint internalFormat = static_cast<GLuint>(data.internalFormat);
	GLuint pixelDataType = static_cast<GLuint>(data.pixelDataType);
	GLuint bindTarget;

	if (isCubeMap)
	{
		assert(store.sideCount == 6);
		bindTarget = GL_TEXTURE_CUBE_MAP;
	}
	else
	{
		assert(store.sideCount == 1);
		bindTarget = GL_TEXTURE_2D;
	}

	GLuint textureID;
	GLCall(glActiveTexture(GL_TEXTURE0));
	glGenTextures(1, &textureID);
	glBindTexture(bindTarget, textureID);

	if (isCubeMap)
	{
		for (unsigned int i = 0; i < store.sideCount; ++i)
		{
			for (unsigned mipMapLevel = 0; mipMapLevel < store.mipmapCounts[i]; ++mipMapLevel)
			{
				auto& image = store.images[i][mipMapLevel];
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipMapLevel, internalFormat, image.width, image.height, 0, format, pixelDataType, image.pixels.get());
			}

		}
	}
	else
	{
		for (unsigned mipMapLevel = 0; mipMapLevel < store.mipmapCounts[0]; ++mipMapLevel)
		{
			auto& image = store.images[0][mipMapLevel];
			glTexImage2D(GL_TEXTURE_2D, mipMapLevel, internalFormat, image.width, image.height, 0, format, pixelDataType, image.pixels.get());
		}
	}

	GLint minFilter = static_cast<GLuint>(data.minFilter);
	GLint magFilter = static_cast<GLuint>(data.magFilter);
	GLint uvTechnique = static_cast<GLuint>(data.uvTechnique);

	glTexParameteri(bindTarget, GL_TEXTURE_WRAP_S, uvTechnique);
	glTexParameteri(bindTarget, GL_TEXTURE_WRAP_T, uvTechnique);
	glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(bindTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);


	if (data.generateMipMaps)
		glGenerateMipmap(bindTarget);

	GLCall(glBindTexture(bindTarget, 0));

	TextureGL* result;

	if (isCubeMap)
	{
		result = new CubeMapGL(textureID);
	}
	else
	{
		result = new TextureGL(textureID);
	}

	result->width = store.images[0][0].width;
	result->height = store.images[0][0].height;

	return result;
}

void nex::TextureGL::setHeight(int height)
{
	this->height = height;
}

void nex::TextureGL::setWidth(int width)
{
	this->width = width;
}

GLuint nex::TextureGL::getTexture() const
{
	return mTextureID;
}

unsigned nex::TextureGL::getHeight() const
{
	return height;
}

unsigned nex::TextureGL::getWidth() const
{
	return width;
}

void nex::TextureGL::setTexture(GLuint id)
{
	mTextureID = id;
}

GLuint nex::TextureGL::getFormat(int numberComponents)
{
	switch (numberComponents) {
		case 4: return GL_RGBA;
		case 3: return GL_RGB;
		case 2: return GL_RG;
		case 1: return GL_RED;
		default: {
			throw_with_trace(runtime_error("TextureManagerGL::getFormat(int): Not supported number of components " + numberComponents));
		}
	}

	// won't be reached
	return GL_FALSE;
}

nex::RenderBufferGL::RenderBufferGL() : TextureGL()
{
}

nex::RenderBufferGL::~RenderBufferGL()
{
	if (mTextureID != GL_FALSE) {
		GLCall(glDeleteRenderbuffers(1, &mTextureID));
		mTextureID = GL_FALSE;
	}
}

nex::RenderBufferGL::RenderBufferGL(GLuint texture) : TextureGL(texture)
{
}