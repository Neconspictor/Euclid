#include <nex/opengl/texture/TextureGL.hpp>
#include <cassert>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Image.hpp>
#include <nex/texture/Texture.hpp>
#include <cassert>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

mat4 nex::CubeMap::rightSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMap::leftSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMap::topSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
mat4 nex::CubeMap::bottomSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
mat4 nex::CubeMap::frontSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMap::backSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f));


const mat4& nex::CubeMap::getViewLookAtMatrixRH(Side side)
{
	switch (side) {
	case Side::POSITIVE_X:
		return rightSide;
	case Side::NEGATIVE_X:
		return leftSide;
	case Side::POSITIVE_Y:
		return topSide;
	case Side::NEGATIVE_Y:
		return bottomSide;
	case Side::NEGATIVE_Z:
		return frontSide;
	case Side::POSITIVE_Z:
		return backSide;
	default:
		throw_with_trace(std::runtime_error("No mapping defined for CubeMap side!"));
	}

	// won't be reached
	return rightSide;
}

nex::CubeMap::CubeMap(std::unique_ptr<TextureImpl> impl) : Texture(std::move(impl))
{
}

nex::CubeMap::CubeMap(unsigned sideWidth, unsigned sideHeight) : Texture(make_unique<CubeMapGL>(sideWidth, sideHeight))
{
}

void nex::CubeMap::generateMipMaps()
{
	auto gl = (CubeMapGL*)getImpl();
	gl->generateMipMaps();
}

unsigned nex::CubeMap::getSideWidth() const
{
	const auto impl = (CubeMapGL*)mImpl.get();
	return impl->getSideWidth();
}

unsigned nex::CubeMap::getSideHeight() const
{
	const auto impl = (CubeMapGL*)mImpl.get();
	return impl->getSideHeight();
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

nex::CubeMapGL::CubeMapGL(unsigned sideWidth, unsigned sideHeight) : TextureGL(), mSideWidth(sideWidth), mSideHeight(sideHeight)
{
}

nex::CubeMapGL::CubeMapGL(GLuint cubeMap, unsigned sideWidth, unsigned sideHeight) : TextureGL(cubeMap), mSideWidth(sideWidth), mSideHeight(sideHeight)
{
}

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

unsigned nex::CubeMapGL::getSideWidth() const
{
	return mSideWidth;
}

unsigned nex::CubeMapGL::getSideHeight() const
{
	return mSideHeight;
}

void nex::CubeMapGL::setSideWidth(unsigned width)
{
	mSideWidth = width;
}

void nex::CubeMapGL::setSideHeight(unsigned height)
{
	mSideHeight = height;
}

void nex::CubeMapGL::setCubeMap(GLuint id)
{
	mTextureID = id;
}

nex::DepthStencilMap::DepthStencilMap(int width, int height, const DepthStencilDesc& desc) : Texture(make_unique<DepthStencilMapGL>(width, height, desc))
{
}


nex::DepthStencilMapGL::DepthStencilMapGL(int width, int height, const DepthStencilDesc& desc) :
	// Note: we don't care about TextureData properties, as we use DepthStencilDesc
	Texture2DGL(GL_FALSE, TextureData(), width, height), mDesc(desc)
{
	GLuint format = translate(mDesc.format);
	GLuint depthType = getDepthType(mDesc.format);
	GLuint dataType = getDataType(mDesc.format);

	GLCall(glGenTextures(1, &mTextureID));
	glBindTexture(GL_TEXTURE_2D, mTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, depthType, dataType, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, translate(mDesc.minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, translate(desc.magFilter));

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, translate(mDesc.wrap));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, translate(mDesc.wrap));

	if (mDesc.wrap == TextureUVTechnique::ClampToBorder)
	{
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (GLfloat*)&mDesc.borderColor.data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);


	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);
}

nex::DepthStencilMapGL::~DepthStencilMapGL()
{
	release();
}

nex::DepthStencilFormat nex::DepthStencilMap::getFormat()
{
	return ((DepthStencilMapGL*)mImpl.get())->mDesc.format;
}

GLuint nex::DepthStencilMapGL::getDepthType(DepthStencilFormat format)
{

	enum DepthStencilTypeGL
	{
		DEPTH_ONLY = GL_DEPTH_COMPONENT,
		STENCIL_ONLY = GL_STENCIL_INDEX,
		DEPTH_STENCIL = GL_DEPTH_STENCIL,
	};

	static DepthStencilTypeGL const table[]
	{
		DEPTH_STENCIL, //DEPTH24_STENCIL8
		DEPTH_STENCIL, //DEPTH32F_STENCIL8
		DEPTH_ONLY,	   //DEPTH_COMPONENT16
		DEPTH_ONLY,	   //DEPTH_COMPONENT24
		DEPTH_ONLY,	   //DEPTH_COMPONENT32
		DEPTH_ONLY,	   //DEPTH_COMPONENT32F
		STENCIL_ONLY, //STENCIL8
	};

	static const unsigned size = (unsigned)DepthStencilFormat::LAST - (unsigned)DepthStencilFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)format];
}

GLuint nex::DepthStencilMapGL::getDataType(DepthStencilFormat format)
{
	enum DepthStencilDataTypeGL
	{
		FLOAT = GL_FLOAT,
		FLOAT_32_UNSIGNED_INT_24_8_REV = GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
		UNSIGNED_INT_24_8 = GL_UNSIGNED_INT_24_8,
		UNSIGNED_INT_8 = GL_UNSIGNED_BYTE,
		UNSIGNED_INT = GL_UNSIGNED_INT,
		UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
		UNSIGNED_INT_24 = GL_UNSIGNED_INT,
	};


	static DepthStencilDataTypeGL const table[]
	{
		UNSIGNED_INT_24_8, //DEPTH24_STENCIL8
		FLOAT_32_UNSIGNED_INT_24_8_REV, //DEPTH32F_STENCIL8
		UNSIGNED_SHORT,	   //DEPTH_COMPONENT16
		UNSIGNED_INT_24,	   //DEPTH_COMPONENT24
		UNSIGNED_INT,	   //DEPTH_COMPONENT32
		FLOAT,	   //DEPTH_COMPONENT32F
		UNSIGNED_INT_8, //STENCIL8
	};

	static const unsigned size = (unsigned)DepthStencilFormat::LAST - (unsigned)DepthStencilFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)format];
}

GLuint nex::DepthStencilMapGL::getAttachmentType(DepthStencilFormat format)
{
	enum AttachmentTypeGL
	{
		DEPTH = GL_DEPTH_ATTACHMENT,
		STENCIL = GL_STENCIL_ATTACHMENT,
		DEPTH_STENCIL = GL_DEPTH_STENCIL_ATTACHMENT,
	};


	static AttachmentTypeGL const table[]
	{
		DEPTH_STENCIL, //DEPTH24_STENCIL8
		DEPTH_STENCIL, //DEPTH32F_STENCIL8
		DEPTH,	   //DEPTH_COMPONENT16
		DEPTH,	   //DEPTH_COMPONENT24
		DEPTH,	   //DEPTH_COMPONENT32
		DEPTH,	   //DEPTH_COMPONENT32F
		STENCIL, //STENCIL8
	};

	static const unsigned size = (unsigned)DepthStencilFormat::LAST - (unsigned)DepthStencilFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)format];
}

const nex::DepthStencilDesc& nex::DepthStencilMapGL::getDescription() const
{
	return mDesc;
}

void nex::DepthStencilMapGL::resize(unsigned width, unsigned height)
{
}

unsigned nex::getComponents(const ColorSpace colorSpace)
{
	static unsigned const table[]
	{
		1,
		1,
		2,
		3,
		4,
		3,
		4,
	};

	static const unsigned size = (unsigned)ColorSpace::LAST - (unsigned)ColorSpace::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)colorSpace];
}


bool nex::isCubeTarget(TextureTarget target)
{
	const auto first = static_cast<unsigned>(TextureTarget::CUBE_POSITIVE_X);
	const auto last = static_cast<unsigned>(TextureTarget::CUBE_NEGATIVE_Z);
	const auto current = static_cast<unsigned>(target);
	return first <= current && last >= current;
}

nex::RenderBuffer* nex::RenderBuffer::create(unsigned width, unsigned height, DepthStencilFormat format)
{
	return new RenderBuffer(width, height, format);
}

nex::RenderBuffer::RenderBuffer(unsigned width, unsigned height, DepthStencilFormat format) : Texture(make_unique<RenderBufferGL>(width, height, format)), mFormat(format)
{
}

nex::DepthStencilFormat nex::RenderBuffer::getFormat() const
{
	return mFormat;
}


nex::RenderBufferGL::RenderBufferGL(GLuint width, GLuint height, DepthStencilFormat format) : TextureGL(GL_FALSE), mFormat(format), mWidth(width), mHeight(height)
{
	GLCall(glGenRenderbuffers(1, &mTextureID));
	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, mTextureID));
	GLCall(glRenderbufferStorage(GL_RENDERBUFFER, translate(mFormat), mWidth, mHeight));
}

nex::RenderBufferGL::~RenderBufferGL()
{
	if (mTextureID != GL_FALSE) {
		GLCall(glDeleteRenderbuffers(1, &mTextureID));
		mTextureID = GL_FALSE;
	}
}

nex::RenderBufferGL::RenderBufferGL(GLuint texture, GLuint width, GLuint height, DepthStencilFormat format)
	: TextureGL(texture), mFormat(format), mWidth(width), mHeight(height)
{
}

nex::DepthStencilFormat nex::RenderBufferGL::getFormat() const
{
	return mFormat;
}

void nex::RenderBufferGL::resize(unsigned width, unsigned height)
{
	mWidth = width;
	mHeight = height;
	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, mTextureID));
	GLCall(glRenderbufferStorage(GL_RENDERBUFFER, translate(mFormat), width, height));
}

nex::Texture::Texture(std::unique_ptr<TextureImpl> impl) : mImpl(std::move(impl))
{
}

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

	if (isCubeMap)
	{
		auto glTexture = std::make_unique<CubeMapGL>(textureID, store.images[0][0].width, store.images[0][0].height);
		auto result = std::make_unique<CubeMap>(std::move(glTexture));
		return result.release();
	}

	auto glTexture = std::make_unique<Texture2DGL>(textureID, data, store.images[0][0].width, store.images[0][0].height);
	auto result = std::make_unique<Texture2D>(std::move(glTexture));
	return result.release();
}

nex::TextureImpl* nex::Texture::getImpl() const
{
	return mImpl.get();
}

void nex::Texture::setImpl(std::unique_ptr<TextureImpl> impl)
{
	mImpl = std::move(impl);
}

nex::TextureGL::TextureGL() : mTextureID(GL_FALSE)
{
}

nex::TextureGL::TextureGL(GLuint texture) : mTextureID(texture)
{
}


nex::TextureGL::~TextureGL()
{
	release();
}

GLuint* nex::TextureGL::getTexture()
{
	return &mTextureID;
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

void nex::TextureGL::release()
{
	if (mTextureID != GL_FALSE) {
		GLCall(glDeleteTextures(1, &mTextureID));
		mTextureID = GL_FALSE;
	}
}

void nex::TextureGL::setTexture(GLuint id)
{
	mTextureID = id;
}

nex::Texture2D::Texture2D(std::unique_ptr<TextureImpl> impl) : Texture(std::move(impl))
{
}

nex::Texture2D::Texture2D(unsigned width, unsigned height, const TextureData& textureData) : Texture(make_unique<Texture2DGL>(GL_FALSE, textureData, width, height))
{
}

nex::Texture2D::Texture2D(unsigned width, unsigned height, const TextureData& textureData, const void* data)
	:
	Texture(make_unique<Texture2DGL>(width, height, textureData, data))
{
}

nex::Texture2D* nex::Texture2D::create(unsigned width, unsigned height, const TextureData& textureData,
	const void* data)
{
	nex::Guard<Texture2D> texture; texture = new Texture2D(width, height, textureData, data);
	return texture.reset();
}

unsigned nex::Texture2D::getWidth() const
{
	auto impl = (Texture2DGL*)mImpl.get();
	return impl->getWidth();
}

unsigned nex::Texture2D::getHeight() const
{
	auto impl = (Texture2DGL*)mImpl.get();
	return impl->getHeight();
}

void nex::Texture2D::resize(unsigned width, unsigned height)
{
	Texture2DGL* impl = (Texture2DGL*)mImpl.get();
	impl->resize(width, height);
}

nex::Texture2DGL::Texture2DGL(GLuint width, GLuint height, const TextureData& textureData, const void* data) :
	TextureGL(GL_FALSE), mWidth(width), mHeight(height), mData(textureData)
{
	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glGenTextures(1, &mTextureID));

	static const GLenum glTarget = GL_TEXTURE_2D;

	GLCall(glBindTexture(glTarget, mTextureID));

	GLCall(glTexImage2D(glTarget, 0, translate(mData.internalFormat), width, height,
		0, translate(mData.colorspace), translate(mData.pixelDataType),
		data));

	if (mData.generateMipMaps)
		GLCall(glGenerateMipmap(glTarget));

	GLCall(glTexParameteri(glTarget, GL_TEXTURE_WRAP_R, translate(mData.wrapR)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, translate(mData.wrapS)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, translate(mData.wrapT)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, translate(mData.minFilter)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, translate(mData.magFilter)));
	GLCall(glTexParameteri(glTarget, GL_TEXTURE_LOD_BIAS, 0.0f));
	GLCall(glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY, 1.0f));

	GLCall(glBindTexture(glTarget, 0));
}

nex::Texture2DGL::Texture2DGL(GLuint texture, const TextureData& textureData, unsigned width, unsigned height)
	: TextureGL(texture), mWidth(width), mHeight(height), mData(textureData)
{

}

unsigned nex::Texture2DGL::getWidth() const
{
	return mWidth;
}

unsigned nex::Texture2DGL::getHeight() const
{
	return mHeight;
}

void nex::Texture2DGL::setHeight(int height)
{
	mHeight = height;
}

void nex::Texture2DGL::setWidth(int width)
{
	mWidth = width;
}

void nex::Texture2DGL::resize(unsigned width, unsigned height)
{
	mWidth = width;
	mHeight = height;

	GLCall(glBindTexture(GL_TEXTURE_2D, mTextureID));

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, translate(mData.internalFormat), width, height,
		0, translate(mData.colorspace), translate(mData.pixelDataType),
		nullptr));

	if (mData.generateMipMaps)
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));
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
		RED_INTEGER,
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

nex::DepthStencilFormatGL nex::translate(nex::DepthStencilFormat format)
{
	static DepthStencilFormatGL const table[]
	{
		DEPTH24_STENCIL8,
		DEPTH32F_STENCIL8,
		DEPTH16,
		DEPTH24,
		DEPTH32,
		DEPTH_COMPONENT32F,
		STENCIL8,
	};

	static const unsigned size = (unsigned)DepthStencilFormat::LAST - (unsigned)DepthStencilFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)format];
}

nex::DepthStencilTypeGL nex::translate(nex::DepthStencilType type)
{
	static DepthStencilTypeGL const typeTable[]
	{
		DEPTH,
		STENCIL,
		DEPTH_STENCIL
	};

	static const unsigned size = (unsigned)DepthStencilType::LAST - (unsigned)DepthStencilType::FIRST + 1;
	static_assert(sizeof(typeTable) / sizeof(typeTable[0]) == size, "GL error: RenderBuffer::Type matching isn't valid!");

	return typeTable[(unsigned)type];
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
