#include <nex/opengl/texture/TextureGL.hpp>
#include <cassert>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Image.hpp>
#include <nex/texture/Texture.hpp>
#include <cassert>
#include <glm/gtc/matrix_transform.inl>
#include <nex/opengl/opengl.hpp>
#include <nex/opengl/RenderBackendGL.hpp>

using namespace std;
using namespace glm;

mat4 nex::CubeMap::rightSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMap::leftSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMap::topSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
mat4 nex::CubeMap::bottomSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
mat4 nex::CubeMap::frontSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMap::backSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f));


const mat4& nex::CubeMap::getViewLookAtMatrixRH(CubeMapSide side)
{
	switch (side) {
	case CubeMapSide::POSITIVE_X:
		return rightSide;
	case CubeMapSide::NEGATIVE_X:
		return leftSide;
	case CubeMapSide::POSITIVE_Y:
		return topSide;
	case CubeMapSide::NEGATIVE_Y:
		return bottomSide;
	case CubeMapSide::NEGATIVE_Z:
		return frontSide;
	case CubeMapSide::POSITIVE_Z:
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

nex::CubeMap::CubeMap(unsigned sideWidth, unsigned sideHeight, const TextureData& data) : Texture(make_unique<CubeMapGL>(sideWidth, sideHeight, data))
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


nex::CubeMapGL::Side nex::CubeMapGL::translate(CubeMapSide side)
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

nex::CubeMapGL::CubeMapGL(unsigned sideWidth, unsigned sideHeight, const TextureData& data) : TextureGL(GL_TEXTURE_CUBE_MAP), mSideWidth(sideWidth), mSideHeight(sideHeight)
{
	auto internalFormat = nex::translate(data.internalFormat);
	auto colorspace = nex::translate(data.colorspace);
	auto pixelDataType = nex::translate(data.pixelDataType);

	TextureGL::generateTexture(&mTextureID, data, mTarget);

	for (int i = 0; i < 6; ++i)
	{
		GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, mSideWidth, mSideHeight, 0, colorspace,
			pixelDataType, nullptr));
	}

	if (data.generateMipMaps)
	{
		generateMipMaps();
	}
}

nex::CubeMapGL::CubeMapGL(GLuint cubeMap, unsigned sideWidth, unsigned sideHeight) : TextureGL(cubeMap, GL_TEXTURE_CUBE_MAP), mSideWidth(sideWidth), mSideHeight(sideHeight)
{
}

void nex::CubeMapGL::generateMipMaps()
{
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID));
	GLCall(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
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

		1,
		1,
		1,
	};

	static const unsigned size = (unsigned)ColorSpace::LAST - (unsigned)ColorSpace::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)colorSpace];
}

nex::RenderBuffer::RenderBuffer(unsigned width, unsigned height, InternFormat format) : Texture(make_unique<RenderBufferGL>(width, height, format))
{
}

nex::InternFormat nex::RenderBuffer::getFormat() const
{
	auto gl = (RenderBufferGL*)mImpl.get();
	return gl->getFormat();
}


nex::RenderBufferGL::RenderBufferGL(GLuint width, GLuint height, InternFormat format) : TextureGL(GL_FALSE, GL_RENDERBUFFER), mFormat(format), mWidth(width), mHeight(height)
{
	GLCall(glGenRenderbuffers(1, &mTextureID));
	GLCall(glBindRenderbuffer(mTarget, mTextureID));
	GLCall(glRenderbufferStorage(mTarget, translate(mFormat), mWidth, mHeight));
}

nex::RenderBufferGL::~RenderBufferGL()
{
	if (mTextureID != GL_FALSE) {
		GLCall(glDeleteRenderbuffers(1, &mTextureID));
		mTextureID = GL_FALSE;
	}
}

nex::RenderBufferGL::RenderBufferGL(GLuint texture, GLuint width, GLuint height, InternFormat format)
	: TextureGL(texture, GL_RENDERBUFFER), mFormat(format), mWidth(width), mHeight(height)
{
}

nex::InternFormat nex::RenderBufferGL::getFormat() const
{
	return mFormat;
}

void nex::RenderBufferGL::resize(unsigned width, unsigned height)
{
	mWidth = width;
	mHeight = height;
	GLCall(glBindRenderbuffer(mTarget, mTextureID));
	GLCall(glRenderbufferStorage(mTarget, translate(mFormat), width, height));
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
	GLCall(glGenTextures(1, &textureID));
	TextureGL::generateTexture(&textureID, data, bindTarget);
	GLCall(glBindTexture(bindTarget, textureID));

	const auto baseWidth = store.images[0]->width;
	const auto baseHeight = store.images[0]->height;
	const auto size = std::min<unsigned>(baseWidth, baseHeight);
	const auto maxMipMapCount = Texture::getMipMapCount(size);

	if (isCubeMap)
	{
		for (unsigned int i = 0; i < store.sideCount; ++i)
		{

			const auto mipMapCount = store.mipmapCounts[i];
			for (unsigned mipMapLevel = 0; mipMapLevel < mipMapCount; ++mipMapLevel)
			{
				auto& image = store.images[i][mipMapLevel];
				GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipMapLevel, internalFormat, image.width, image.height, 0, format, pixelDataType, image.pixels.get()));
			}

			// Has the texture non base mipmaps?
			if (mipMapCount > 1)
			{
				// NOTE: opengl needs a complete mipmap chain in order to work properly; otherwise mipmaps aren't used
				for (unsigned mipMapLevel = store.mipmapCounts[i]; mipMapLevel < maxMipMapCount; ++mipMapLevel)
				{
					const auto divisor = std::pow<unsigned>(2, mipMapLevel);
					GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipMapLevel, internalFormat, baseWidth / divisor, baseHeight / divisor, 0, format, pixelDataType, nullptr));
				}
			}
		}
	}
	else
	{

		const auto mipMapCount = store.mipmapCounts[0];

		for (unsigned mipMapLevel = 0; mipMapLevel < mipMapCount; ++mipMapLevel)
		{
			auto& image = store.images[0][mipMapLevel];
			GLCall(glTexImage2D(GL_TEXTURE_2D, mipMapLevel, internalFormat, image.width, image.height, 0, format, pixelDataType, image.pixels.get()));
		}

		// Has the texture non base mipmaps?
		if (mipMapCount > 1)
		{
			// NOTE: opengl needs a complete mipmap chain in order to work properly; otherwise mipmaps aren't used
			for (unsigned mipMapLevel = mipMapCount; mipMapLevel < maxMipMapCount; ++mipMapLevel)
			{
				const auto divisor = std::pow<unsigned>(2, mipMapLevel);
				GLCall(glTexImage2D(GL_TEXTURE_2D, mipMapLevel, internalFormat, baseWidth / divisor, baseHeight / divisor, 0, format, pixelDataType, nullptr));
			}
		}
	}


	if (data.generateMipMaps &&  store.mipmapCounts[0] == 1)
	{
		GLCall(glGenerateMipmap(bindTarget));
	}

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

std::unique_ptr<nex::Texture> nex::Texture::createView(Texture* original, TextureTarget target, unsigned minLevel, unsigned numLevel,
	unsigned minLayer, unsigned numLayers, const TextureData& data)
{
	auto impl = TextureGL::createView((TextureGL*)original->getImpl(), target, minLevel, numLevel, minLayer, numLayers, data);
	return make_unique<Texture>(std::move(impl));
}

nex::TextureImpl* nex::Texture::getImpl() const
{
	return mImpl.get();
}

unsigned nex::Texture::getMipMapCount(unsigned levelZeroMipMap)
{
	unsigned count = 0;
	while(levelZeroMipMap > 0)
	{
		levelZeroMipMap /= 2;
		++count;
	}

	return count;
}

void nex::Texture::readback(TextureTarget target, unsigned mipmapLevel, ColorSpace format, PixelDataType type, void * dest, CubeMapSide side)
{
	auto gl = (TextureGL*)getImpl();
	gl->readback(target, mipmapLevel, format, type, dest, side);
}

void nex::Texture::setImpl(std::unique_ptr<TextureImpl> impl)
{
	mImpl = std::move(impl);
}

nex::TextureGL::TextureGL(GLuint target) : mTextureID(GL_FALSE), mTarget(target)
{
}

nex::TextureGL::TextureGL(GLuint texture, GLuint target) : mTextureID(texture), mTarget(target)
{
}


nex::TextureGL::~TextureGL()
{
	release();
}

std::unique_ptr<nex::TextureGL> nex::TextureGL::createView(TextureGL* original, TextureTarget target, unsigned minLevel, unsigned numLevel,
	unsigned minLayer, unsigned numLayers, const TextureData& data)
{
	auto result = make_unique<TextureGL>(original->getTarget());
	// TODO check whether target and the target of the original texture are compatible!
	const GLenum targetGL = translate(target);

	const GLenum internalFormat = translate(data.internalFormat);
	
	GLCall(glGenTextures(1, result->getTexture()));
	const GLuint textureID = *result->getTexture();

	GLCall(glTextureView(textureID, targetGL, *original->getTexture(), internalFormat, minLevel, numLevel, minLayer, numLayers));
	applyTextureData(textureID, data, targetGL);
	return result;
}

void nex::TextureGL::generateTexture(GLuint* out, const BaseTextureDesc& desc, GLenum target)
{
	GLCall(glActiveTexture(GL_TEXTURE0 + desc.textureIndex));
	GLCall(glGenTextures(1, out));
	applyTextureData(*out, desc, target);
}

void nex::TextureGL::applyTextureData(GLuint texture, const BaseTextureDesc& desc, GLenum target)
{
	GLCall(glBindTexture(target, texture));

	GLCall(glTexParameteri(target, GL_TEXTURE_WRAP_R, translate(desc.wrapR)));
	GLCall(glTexParameteri(target, GL_TEXTURE_WRAP_S, translate(desc.wrapS)));
	GLCall(glTexParameteri(target, GL_TEXTURE_WRAP_T, translate(desc.wrapT)));
	GLCall(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, translate(desc.minFilter)));
	GLCall(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, translate(desc.magFilter)));
	GLCall(glTexParameteri(target, GL_TEXTURE_LOD_BIAS, desc.biasLOD));
	GLCall(glTexParameteri(target, GL_TEXTURE_MIN_LOD, desc.minLOD));
	GLCall(glTexParameteri(target, GL_TEXTURE_MAX_LOD, desc.maxLOD));
	GLCall(glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, desc.lodBaseLevel));
	GLCall(glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, desc.lodMaxLevel));
	GLCall(glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, desc.maxAnisotropy));

	//swizzle
	if (desc.useSwizzle)
	{
		int swizzle[4];
		swizzle[0] = translate(desc.swizzle.r);
		swizzle[1] = translate(desc.swizzle.g);
		swizzle[2] = translate(desc.swizzle.b);
		swizzle[3] = translate(desc.swizzle.a);

		GLCall(glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA, swizzle));
	}

	// border color
	GLCall(glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, (float*)&desc.borderColor.data));


	// depth comparison
	if (desc.useDepthComparison)
	{
		GLCall(glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
		GLCall(glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, translate(desc.compareFunc)));
	}
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

GLuint* nex::TextureGL::getTexture()
{
	return &mTextureID;
}

void nex::TextureGL::readback(TextureTarget target, unsigned mipmapLevel, ColorSpace format, PixelDataType type,
	void* dest, CubeMapSide side)
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GLCall(glActiveTexture(GL_TEXTURE0));
	if (target == TextureTarget::CUBE_MAP)
	{
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID));
		GLCall(glGetTexImage(CubeMapGL::translate(side), mipmapLevel, translate(format), translate(type), dest));
	}
	else
	{
		GLCall(glBindTexture(translate(target), mTextureID));
		GLCall(glGetTexImage(translate(target), mipmapLevel, translate(format), translate(type), dest));
	}
}

void nex::TextureGL::resizeTexImage2D(GLuint textureID, GLenum target, GLint level, unsigned width, unsigned height, GLenum  colorspace,
	GLint internalFormat, GLenum  pixelDataType, bool generateMipMaps, const void* data)
{
	GLCall(glBindTexture(target, textureID));

	GLCall(glTexImage2D(target, level, internalFormat, width, height, 0, colorspace, pixelDataType, data));

	if (generateMipMaps)
	{
		GLCall(glGenerateMipmap(target));
	}
}

void nex::TextureGL::resizeTexImage3D(GLuint textureID, GLenum target, GLint level, unsigned width, unsigned height,
	unsigned depth, GLenum colorspace, GLint internalFormat, GLenum pixelDataType, bool generateMipMaps,
	const void* data)
{
	GLCall(glBindTexture(target, textureID));

	GLCall(glTexImage3D(target, level, internalFormat, width, height, depth, 0, colorspace, pixelDataType, data));

	if (generateMipMaps)
	{
		GLCall(glGenerateMipmap(target));
	}
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

GLuint nex::TextureGL::getTarget() const
{
	return mTarget;
}

nex::Texture2D::Texture2D(std::unique_ptr<TextureImpl> impl) : Texture(std::move(impl))
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
	TextureGL(GL_FALSE, GL_TEXTURE_2D), mWidth(width), mHeight(height), mData(textureData)
{
	TextureGL::generateTexture(&mTextureID, textureData, mTarget);

	GLCall(glTexImage2D(mTarget, 0, translate(mData.internalFormat), width, height,
		0, translate(mData.colorspace), translate(mData.pixelDataType),
		data));


	if (mData.generateMipMaps)
	{
		GLCall(glGenerateMipmap(mTarget));
	}
}

nex::Texture2DGL::Texture2DGL(GLuint texture, const TextureData& textureData, unsigned width, unsigned height)
	: TextureGL(texture, GL_TEXTURE_2D), mWidth(width), mHeight(height), mData(textureData)
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

	resizeTexImage2D(mTextureID, mTarget, 0, mWidth, mHeight, translate(mData.colorspace), translate(mData.internalFormat),
		translate(mData.pixelDataType), mData.generateMipMaps, nullptr);
}


nex::Texture2DMultisample::Texture2DMultisample(std::unique_ptr<TextureImpl> impl) : Texture2D(std::move(impl))
{
}

nex::Texture2DMultisample::Texture2DMultisample(unsigned width, unsigned height, const TextureData& textureData,
	unsigned samples) : Texture2D(make_unique<Texture2DMultisampleGL>(width, height, textureData, samples))
{
}

void nex::Texture2DMultisample::resize(unsigned width, unsigned height)
{
	auto impl = (Texture2DMultisampleGL*)mImpl.get();
	impl->resize(width, height);
}

unsigned nex::Texture2DMultisample::getSamples() const
{
	auto impl = (Texture2DMultisampleGL*)mImpl.get();
	return impl->getSamples();
}

nex::Texture2DMultisampleGL::Texture2DMultisampleGL(GLuint width, GLuint height, const TextureData& textureData,
	unsigned samples) : Texture2DGL(GL_FALSE, textureData, width, height), mSamples(samples)
{
	mTarget = TEXTURE2D_MULTISAMPLE;
	TextureGL::generateTexture(&mTextureID, textureData, mTarget);
	GLCall(glTexImage2DMultisample(mTarget, mSamples, translate(mData.internalFormat), mWidth, mHeight, GL_TRUE));

	if (mData.generateMipMaps)
	{
		GLCall(glGenerateMipmap(mTarget));
	}
}

nex::Texture2DMultisampleGL::Texture2DMultisampleGL(GLuint texture, const TextureData& textureData, unsigned samples,
	unsigned width, unsigned height) : Texture2DGL(texture, textureData, width, height), mSamples(samples)
{
	mTarget = TEXTURE2D_MULTISAMPLE;
}

void nex::Texture2DMultisampleGL::resize(unsigned width, unsigned height)
{
	mWidth = width;
	mHeight = height;
	GLCall(glTexImage2DMultisample(mTarget, mSamples, translate(mData.internalFormat), mWidth, mHeight, GL_TRUE));

	if (mData.generateMipMaps)
	{
		GLCall(glGenerateMipmap(mTarget));
	}
}

unsigned nex::Texture2DMultisampleGL::getSamples() const
{
	return mSamples;
}

nex::Texture2DArray::Texture2DArray(std::unique_ptr<TextureImpl> impl) : Texture(std::move(impl))
{
}

nex::Texture2DArray::Texture2DArray(unsigned width, unsigned height, unsigned size, bool immutableStorage, const TextureData& textureData,
	const void* data) :
	Texture(make_unique<Texture2DArrayGL>(width, height, size, immutableStorage, textureData, data))
{
}

void nex::Texture2DArray::resize(unsigned width, unsigned height, unsigned size)
{
	auto impl = (Texture2DArrayGL*)mImpl.get();
	impl->resize(width, height, size);
}

unsigned nex::Texture2DArray::getWidth() const
{
	auto impl = (Texture2DArrayGL*)mImpl.get();
	return impl->getWidth();
}

unsigned nex::Texture2DArray::getHeight() const
{
	auto impl = (Texture2DArrayGL*)mImpl.get();
	return impl->getHeight();
}

unsigned nex::Texture2DArray::getSize() const
{
	auto impl = (Texture2DArrayGL*)mImpl.get();
	return impl->getSize();
}


nex::Texture2DArrayGL::Texture2DArrayGL(GLuint width, GLuint height, GLuint size, bool immutableStorage, const TextureData& textureData,
	const void* data) :
	TextureGL(GL_FALSE, GL_TEXTURE_2D_ARRAY), mWidth(width), mHeight(height), mSize(size), mData(textureData)
{
	TextureGL::generateTexture(&mTextureID, textureData, mTarget);

	//GLCall(glBindTexture(mTarget, mTextureID));

	if (immutableStorage)
	{
		GLCall(glTexStorage3D(mTarget, 1, translate(mData.internalFormat), width, height, size));
		GLCall(glTexSubImage3D(mTarget, 0, 0, 0, 0, width, height, size, translate(mData.colorspace), translate(mData.pixelDataType), data));
	} else
	{
		resizeTexImage3D(mTextureID,
			mTarget,
			0,
			mWidth,
			mHeight,
			mSize,
			translate(mData.colorspace),
			translate(mData.internalFormat),
			translate(mData.pixelDataType),
			mData.generateMipMaps,
			data);
	}
}

nex::Texture2DArrayGL::Texture2DArrayGL(GLuint texture, const TextureData& textureData, unsigned width, unsigned height, unsigned size)
	: TextureGL(texture, GL_TEXTURE_2D), mWidth(width), mHeight(height), mSize(size), mData(textureData)
{
}

unsigned nex::Texture2DArrayGL::getWidth() const
{
	return mWidth;
}

unsigned nex::Texture2DArrayGL::getHeight() const
{
	return mHeight;
}

unsigned nex::Texture2DArrayGL::getSize() const
{
	return mSize;
}

void nex::Texture2DArrayGL::setHeight(unsigned height)
{
	mHeight = height;
}

void nex::Texture2DArrayGL::setWidth(unsigned width)
{
	mWidth = width;
}

void nex::Texture2DArrayGL::setSize(unsigned size)
{
	mSize = size;
}

void nex::Texture2DArrayGL::resize(unsigned width, unsigned height, unsigned size)
{
	///TODO
	assert(false);
	return;
	mWidth = width;
	mHeight = height;
	mSize = size;

	resizeTexImage3D(mTextureID, 
		mTarget, 
		0, 
		mWidth, 
		mHeight, 
		mSize, 
		translate(mData.colorspace), 
		translate(mData.internalFormat),
		translate(mData.pixelDataType), 
		mData.generateMipMaps, 
		nullptr);
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
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureAccess and TextureAccessGL don't match!");

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
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: Channel and ChannelGL don't match!");

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
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureFilter and TextureFilterGL don't match!");

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
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureUVTechnique and TextureUVTechniqueGL don't match!");

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

		DEPTH,
		STENCIL,
		DEPTH_STENCIL,
	};

	static const unsigned size = (unsigned)ColorSpace::LAST - (unsigned)ColorSpace::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: ColorSpace and ColorSpaceGL don't match!");

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
		RGBA16_SNORM,
		RGBA32F,
		RGBA32I,
		RGBA32UI,

		// srgb formats
		SRGB8,
		SRGBA8,

		DEPTH24_STENCIL8,
		DEPTH32F_STENCIL8,
		DEPTH16,
		DEPTH24,
		DEPTH32,
		DEPTH_COMPONENT32F,
		STENCIL8,
	};

	static const unsigned size = (unsigned)InternFormat::LAST - (unsigned)InternFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: InternFormat and InternFormatGL don't match!");

	return table[(unsigned)format];
}

nex::PixelDataTypeGL nex::translate(nex::PixelDataType dataType)
{
	static PixelDataTypeGL const table[]
	{
		FLOAT,
		UBYTE,
		UINT,
		SHORT,

		UNSIGNED_INT_24_8,
		FLOAT_32_UNSIGNED_INT_24_8_REV,
		UNSIGNED_SHORT,
		UNSIGNED_INT_24,
		UNSIGNED_INT_8,
	};

	static const unsigned size = (unsigned)PixelDataType::LAST - (unsigned)PixelDataType::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: PixelDataType and PixelDataTypeGL don't match!");

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
		TEXTURE2D_MULTISAMPLE,

		// 3D
		TEXTURE2D_ARRAY,
		TEXTURE2D_MULTISAMPLE_ARRAY,
		TEXTURE3D,

		// cubemap
		CUBE_MAP,
	};

	static const unsigned size = (unsigned)TextureTarget::LAST - (unsigned)TextureTarget::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureTarget and TextureTargetGl don't match!");

	return table[(unsigned)target];
}