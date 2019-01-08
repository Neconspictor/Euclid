#include <nex/opengl/texture/TextureGL.hpp>
#include <cassert>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/texture/Image.hpp>

using namespace std;
using namespace glm;

mat4 nex::CubeMapGL::rightSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMapGL::leftSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMapGL::topSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
mat4 nex::CubeMapGL::bottomSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
mat4 nex::CubeMapGL::frontSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMapGL::backSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f));


nex::CubeMap* nex::CubeMap::create()
{
	return new CubeMap();
}

void nex::CubeMap::generateMipMaps()
{
	CubeMapGL* gl = (CubeMapGL*)getImpl();
	gl->generateMipMaps();
}

nex::CubeMap::CubeMap() : Texture(new CubeMapGL())
{
}


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

nex::CubeMapGL::Side nex::CubeMapGL::translate(CubeMap::Side side)
{
	static Side const table[]
	{
			POSITIVE_X,
			NEGATIVE_X,
			POSITIVE_Y,
			NEGATIVE_Y,
			POSITIVE_Z,
			NEGATIVE_Z,
	};

	//static const unsigned size = (unsigned)CubeMap::Side::LAST - (unsigned)TextureFilter::FIRST + 1;
	//static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)side];
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

bool nex::isNoStencilFormat(nex::DepthStencil format)
{
	return format == DepthStencil::NONE ||
		format == DepthStencil::DEPTH24;
}

GLuint nex::translate(bool boolean)
{
	return boolean ? GL_TRUE : GL_FALSE;
}

nex::TextureAccessGL nex::translate(nex::TextureAccess accessType)
{
	static TextureAccessGL const table[]
	{
		READ_ONLY,
		READ_WRITE,
		WRITE_ONLY,
	};

	static const unsigned size = (unsigned)TextureAccess::LAST - (unsigned)TextureAccess::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)accessType];
}

nex::ChannelGL nex::translate(nex::Channel channel)
{
	static ChannelGL const table[]
	{
		RED,
		GREEN,
		BLUE,
		ALPHA,
	};

	static const unsigned size = (unsigned)Channel::LAST - (unsigned)Channel::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)channel];
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
		CUBE_MAP,
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

nex::DepthStencilGL nex::translate(nex::DepthStencil depth)
{
	static DepthStencilGL const table[]
	{
		NONE,
		DEPTH24,
		DEPTH24_STENCIL8,
		DEPTH32F_STENCIL8,
	};

	static const unsigned size = (unsigned)DepthStencil::LAST - (unsigned)DepthStencil::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)depth];
}

nex::TextureGL::TextureGL(): mTextureID(GL_FALSE), width(0), height(0)
{
}

nex::TextureGL::TextureGL(GLuint texture) : mTextureID(texture), width(0), height(0)
{
}


nex::TextureGL::~TextureGL()
{

}

void nex::TextureGL::release()
{
	if (mTextureID != GL_FALSE) {
		GLCall(glDeleteTextures(1, &mTextureID));
		mTextureID = GL_FALSE;
	}
}

nex::Texture* nex::Texture::create()
{
	Guard<TextureGL> gl(new TextureGL());
	return new Texture(gl.reset());
}

/*
nex::Texture* nex::Texture::create(TextureTarget target, const TextureData& data)
{
	nex::Guard<Texture> texture(Texture::create());
	GLuint textureID;
	GLCall(glActiveTexture(GL_TEXTURE0));
	glGenTextures(1, &textureID);

	const GLenum glTarget = translate(target);

	glBindTexture(glTarget, textureID);

	if (data.generateMipMaps)
		glGenerateMipmap(glTarget);

	glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, translate(data.uvTechnique));
	glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, translate(data.uvTechnique));
	glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, translate(data.minFilter));
	glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, translate(data.magFilter));
	glTexParameteri(glTarget, GL_TEXTURE_LOD_BIAS, 0.0f);
	glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);

	GLCall(glBindTexture(glTarget, 0));

	((TextureGL*)texture->getImpl())->setTexture(textureID);

	return texture.reset();
}*/

nex::Texture* nex::Texture::createFromImage(const StoreImage& store, const TextureData& data, bool isCubeMap)
{
	GLuint format = translate(data.colorspace);
	GLuint internalFormat = translate(data.internalFormat);
	GLuint pixelDataType = translate(data.pixelDataType);
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

	GLint minFilter = translate(data.minFilter);
	GLint magFilter = translate(data.magFilter);
	GLint wrapR = translate(data.wrapR);
	GLint wrapS = translate(data.wrapS);
	GLint wrapT = translate(data.wrapT);

	glTexParameteri(bindTarget, GL_TEXTURE_WRAP_R, wrapR);
	glTexParameteri(bindTarget, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(bindTarget, GL_TEXTURE_WRAP_T, wrapT);
	glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(bindTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);


	if (data.generateMipMaps)
		glGenerateMipmap(bindTarget);

	GLCall(glBindTexture(bindTarget, 0));

	Guard<TextureGL> glTexture;
	nex::Guard<Texture> texture;

	if (isCubeMap)
	{
		glTexture = new CubeMapGL(textureID);
		texture = CubeMap::create();
	}
	else
	{
		glTexture = new TextureGL(textureID);
		texture = Texture::create();
	}


	glTexture->width = store.images[0][0].width;
	glTexture->height = store.images[0][0].height;

	Texture* result = texture.reset();
	result->mImpl = glTexture.reset();

	return result;
}

nex::Texture* nex::Texture::createTexture2D(unsigned width, unsigned height, const TextureData& textureData,
	const void* data)
{
	nex::Guard<Texture> texture(Texture::create());
	GLuint textureID;
	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glGenTextures(1, &textureID));

	static const GLenum glTarget = GL_TEXTURE_2D;

	GLCall(glBindTexture(glTarget, textureID));

	GLCall(glTexImage2D(glTarget, 0, translate(textureData.internalFormat), width, height,
		0, translate(textureData.colorspace), translate(textureData.pixelDataType), 
		data));

	if (textureData.generateMipMaps)
		GLCall(glGenerateMipmap(glTarget));

	GLCall(glTexParameteri(glTarget, GL_TEXTURE_WRAP_R, translate(textureData.wrapR)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, translate(textureData.wrapS)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, translate(textureData.wrapT)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, translate(textureData.minFilter)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, translate(textureData.magFilter)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_LOD_BIAS, 0.0f));
	GLCall(glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY, 1.0f));

	GLCall(glBindTexture(glTarget, 0));

	((TextureGL*)texture->getImpl())->setTexture(textureID);
	((TextureGL*)texture->getImpl())->setWidth(width);
	((TextureGL*)texture->getImpl())->setHeight(height);

	texture->setWidth(width);
	texture->setHeight(height);

	return texture.reset();
}

void nex::TextureGL::setHeight(int height)
{
	this->height = height;
}

void nex::TextureGL::setWidth(int width)
{
	this->width = width;
}

GLuint* nex::TextureGL::getTexture()
{
	return &mTextureID;
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

nex::RenderBuffer* nex::RenderBuffer::create()
{
	return new RenderBuffer();
}

nex::RenderBuffer::RenderBuffer() : Texture(new RenderBufferGL())
{
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