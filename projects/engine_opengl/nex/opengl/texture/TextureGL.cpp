#include <nex/opengl/texture/TextureGL.hpp>
#include <cassert>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Image.hpp>
#include <nex/texture/Texture.hpp>
#include <cassert>
#include <glm/gtc/matrix_transform.inl>
#include <nex/opengl/opengl.hpp>
#include <nex/opengl/RenderBackendGL.hpp>
#include "nex/opengl/CacheGL.hpp"

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

nex::CubeMap::CubeMap(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::CubeMap::CubeMap(unsigned sideWidth, unsigned sideHeight, const TextureData& data) : Texture(make_unique<CubeMapGL>(sideWidth, sideHeight, data))
{
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
			Side::POSITIVE_X,
			Side::NEGATIVE_X,
			Side::POSITIVE_Y,
			Side::NEGATIVE_Y,
			Side::POSITIVE_Z,
			Side::NEGATIVE_Z,
	};

	//static const unsigned size = (unsigned)CubeMap::Side::LAST - (unsigned)TextureFilter::FIRST + 1;
	//static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)side];
}

nex::CubeMapGL::CubeMapGL(unsigned sideWidth, unsigned sideHeight, const TextureData& data) : Impl(TextureTargetGl::CUBE_MAP), mSideWidth(sideWidth), mSideHeight(sideHeight)
{
	const auto internalFormat = (GLenum)nex::translate(data.internalFormat);

	generateTexture(&mTextureID, data, (GLenum)mTarget);
	resizeTexImage2D(mTextureID, 1, mSideWidth, mSideHeight, internalFormat, data.generateMipMaps);
	if (data.generateMipMaps) generateMipMaps();
}

nex::CubeMapGL::CubeMapGL(GLuint cubeMap, unsigned sideWidth, unsigned sideHeight) : Impl(cubeMap, TextureTargetGl::CUBE_MAP), mSideWidth(sideWidth), mSideHeight(sideHeight)
{
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


nex::RenderBufferGL::RenderBufferGL(GLuint width, GLuint height, InternFormat format) : Impl(GL_FALSE, TextureTargetGl::RENDERBUFFER), mFormat(format), mWidth(width), mHeight(height)
{
	GLCall(glGenRenderbuffers(1, &mTextureID));
	GLCall(glNamedRenderbufferStorage(mTextureID, (GLenum)translate(mFormat), width, height));
}

nex::RenderBufferGL::~RenderBufferGL()
{
	if (mTextureID != GL_FALSE) {
		GLCall(glDeleteRenderbuffers(1, &mTextureID));
		mTextureID = GL_FALSE;
	}
}

nex::RenderBufferGL::RenderBufferGL(GLuint texture, GLuint width, GLuint height, InternFormat format)
	: Impl(texture, TextureTargetGl::RENDERBUFFER), mFormat(format), mWidth(width), mHeight(height)
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
	GLCall(glNamedRenderbufferStorage(mTextureID, (GLenum)translate(mFormat), width, height));
}

nex::Texture::~Texture() = default;

nex::Texture::Texture(std::unique_ptr<Impl> impl) : mImpl(std::move(impl))
{
}

nex::Texture* nex::Texture::createFromImage(const StoreImage& store, const TextureData& data, bool isCubeMap)
{
	const auto format = (GLenum)translate(data.colorspace);
	const auto internalFormat = (GLenum)translate(data.internalFormat);
	const auto pixelDataType = (GLenum)translate(data.pixelDataType);
	GLenum bindTarget;

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

	auto dataCopy = data;
	if (store.mipmapCounts[0] > 0)
	{
		dataCopy.generateMipMaps = true;
	}

	GLuint textureID;
	Impl::generateTexture(&textureID, dataCopy, bindTarget);

	const auto baseWidth = store.images[0]->width;
	const auto baseHeight = store.images[0]->height;
	const auto size = std::min<unsigned>(baseWidth, baseHeight);
	const auto maxMipMapCount = Texture::getMipMapCount(size);

	// allocate texture storage
	Impl::resizeTexImage2D(textureID, 1, baseWidth, baseHeight, internalFormat, dataCopy.generateMipMaps);

	if (isCubeMap)
	{
		for (unsigned int i = 0; i < store.sideCount; ++i)
		{

			const auto mipMapCount = store.mipmapCounts[i];
			for (unsigned mipMapLevel = 0; mipMapLevel < mipMapCount; ++mipMapLevel)
			{
				auto& image = store.images[i][mipMapLevel];
				GLCall(glTextureSubImage3D(textureID, 
					mipMapLevel,
					0, 0,
					i, // zoffset specifies the cubemap side
					image.width, image.height,
					1, // depth specifies the number of sides to be updated
					format,
					pixelDataType,
					image.pixels.get()));
			}
		}
	}
	else
	{
		const auto mipMapCount = store.mipmapCounts[0];

		for (unsigned mipMapLevel = 0; mipMapLevel < mipMapCount; ++mipMapLevel)
		{
			auto& image = store.images[0][mipMapLevel];
			GLCall(glTextureSubImage2D(textureID,
				mipMapLevel,
				0, 0,
				image.width, image.height,
				format,
				pixelDataType,
				image.pixels.get()));
		}
	}


	if (data.generateMipMaps &&  store.mipmapCounts[0] == 1)
	{
		GLCall(glGenerateTextureMipmap(textureID));
	}

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
	auto impl = Impl::createView(original->getImpl(), target, minLevel, numLevel, minLayer, numLayers, data);
	return make_unique<Texture>(std::move(impl));
}

void nex::Texture::generateMipMaps()
{
	//GLCall(glActiveTexture(GL_TEXTURE0));
	//GLCall(glBindTexture((GLenum)mImpl->mTarget, mImpl->mTextureID));
	//GLCall(glBindSampler(0, GL_FALSE));

	mImpl->generateMipMaps();
}

nex::Texture::Impl* nex::Texture::getImpl() const
{
	return mImpl.get();
}

unsigned nex::Texture::getMipMapCount(unsigned levelZeroMipMap)
{
	return std::log2<>(levelZeroMipMap) + 1;
}

void nex::Texture::readback(unsigned mipmapLevel, ColorSpace format, PixelDataType type, void * dest, size_t destBufferSize)
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GlobalCacheGL::get()->UseProgram(0);
	GLCall(glGetTextureImage(mImpl->mTextureID, mipmapLevel, (GLenum)translate(format), (GLenum)translate(type), destBufferSize, dest));
	GLCall(glFinish());
}

void nex::Texture::setImpl(std::unique_ptr<Impl> impl)
{
	mImpl = std::move(impl);
}

nex::Texture::Impl::Impl(TextureTargetGl target) : mTextureID(GL_FALSE), mTarget(target)
{
}

nex::Texture::Impl::Impl(GLuint texture, TextureTargetGl target) : mTextureID(texture), mTarget(target)
{
}


nex::Texture::Impl::~Impl()
{
	release();
}

void nex::Texture::Impl::generateMipMaps()
{
	GLCall(glTextureParameteri(mTextureID, GL_TEXTURE_BASE_LEVEL, 0));
	GLCall(glTextureParameteri(mTextureID, GL_TEXTURE_MAX_LEVEL, 1000));
	GLCall(glGenerateTextureMipmap(mTextureID));
}

std::unique_ptr<nex::Texture::Impl> nex::Texture::Impl::createView(Impl* original, TextureTarget target, unsigned minLevel, unsigned numLevel,
	unsigned minLayer, unsigned numLayers, const TextureData& data)
{
	auto result = make_unique<Impl>(original->getTarget());
	// TODO check whether target and the target of the original texture are compatible!
	const auto targetGL = (GLenum)translate(target);

	const auto internalFormat = (GLenum)translate(data.internalFormat);


	GLCall(glGenTextures(1, result->getTexture())); // Note: we mustn't bind the texture to a target (necessary for glTextureView)
	//GLCall(glCreateTextures(targetGL, 1, result->getTexture()));
	const GLuint textureID = *result->getTexture();

	GLCall(glTextureView(textureID, targetGL, *original->getTexture(), internalFormat, minLevel, numLevel, minLayer, numLayers));
	applyTextureData(textureID, data);
	return result;
}

void nex::Texture::Impl::generateTexture(GLuint* out, const BaseTextureDesc& desc, GLenum target)
{
	//GLCall(glActiveTexture(GL_TEXTURE0 + desc.textureIndex));
	//GLCall(glGenTextures(1, out));
	GLCall(glCreateTextures(target, 1, out));
	applyTextureData(*out, desc);
}

void nex::Texture::Impl::applyTextureData(GLuint texture, const BaseTextureDesc& desc)
{
	//GLCall(glBindTexture(target, texture));

	GLCall(glTextureParameteri(texture, GL_TEXTURE_WRAP_R, (GLenum)translate(desc.wrapR)));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_WRAP_S, (GLenum)translate(desc.wrapS)));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_WRAP_T, (GLenum)translate(desc.wrapT)));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, (GLenum)translate(desc.minFilter)));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, (GLenum)translate(desc.magFilter)));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_LOD_BIAS, desc.biasLOD));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_MIN_LOD, desc.minLOD));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_MAX_LOD, desc.maxLOD));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_BASE_LEVEL, desc.lodBaseLevel));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, desc.lodMaxLevel));
	GLCall(glTextureParameteri(texture, GL_TEXTURE_MAX_ANISOTROPY, desc.maxAnisotropy));

	//swizzle
	if (desc.useSwizzle)
	{
		int swizzle[4];
		swizzle[0] = (GLenum)translate(desc.swizzle.r);
		swizzle[1] = (GLenum)translate(desc.swizzle.g);
		swizzle[2] = (GLenum)translate(desc.swizzle.b);
		swizzle[3] = (GLenum)translate(desc.swizzle.a);

		GLCall(glTextureParameteriv(texture, GL_TEXTURE_SWIZZLE_RGBA, swizzle));
	}

	// border color
	GLCall(glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, (float*)&desc.borderColor.data));


	// depth comparison
	if (desc.useDepthComparison)
	{
		GLCall(glTextureParameteri(texture, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
		GLCall(glTextureParameteri(texture, GL_TEXTURE_COMPARE_FUNC, (GLenum)translate(desc.compareFunc)));
	}
}

GLuint nex::Texture::Impl::getFormat(int numberComponents)
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

GLuint* nex::Texture::Impl::getTexture()
{
	return &mTextureID;
}

void nex::Texture::Impl::resizeTexImage2D(GLuint textureID, GLint levels, unsigned width, unsigned height,
	GLint internalFormat, bool generateMipMaps)
{
	//GLCall(glBindTexture(target, textureID)); //glTextureStorage2D
	//GLCall(glTexImage2D(target, level, internalFormat, width, height, 0, colorspace, pixelDataType, data));

	if (generateMipMaps)
	{
		levels = std::log2<>(std::min<>(width, height)) + 1;
	}

	GLCall(glTextureStorage2D(textureID, levels, internalFormat, width, height));
}

void nex::Texture::Impl::resizeTexImage3D(GLuint textureID, GLint levels, unsigned width, unsigned height,
	unsigned depth, GLint internalFormat, bool generateMipMaps)
{
	//GLCall(glBindTexture(target, textureID));
	//GLCall(glTexImage3D(target, level, internalFormat, width, height, depth, 0, colorspace, pixelDataType, data));


	if (generateMipMaps)
	{
		levels = std::log2<>(std::min<>(width, height)) + 1;
	}

	GLCall(glTextureStorage3D(textureID, levels, internalFormat, width, height, depth));
}

void nex::Texture::Impl::release()
{
	if (mTextureID != GL_FALSE) {
		GLCall(glDeleteTextures(1, &mTextureID));
		mTextureID = GL_FALSE;
	}
}

void nex::Texture::Impl::setTexture(GLuint id)
{
	mTextureID = id;
}

nex::TextureTargetGl nex::Texture::Impl::getTarget() const
{
	return mTarget;
}

nex::Texture2D::Texture2D(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::Texture2D::Texture2D(unsigned width, unsigned height, const TextureData& textureData, const void* data)
	:
	Texture(make_unique<Texture2DGL>(width, height, textureData, data))
{
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
	Impl(GL_FALSE, TextureTargetGl::TEXTURE2D), mWidth(width), mHeight(height), mData(textureData)
{
	generateTexture(&mTextureID, textureData, (GLenum)mTarget);


	resizeTexImage2D(mTextureID,
		1, // levels will be calculated automatically when generateMipMaps is true 
		width, height, 
		(GLenum)translate(mData.internalFormat),
		mData.generateMipMaps // specifies the storage for mipmaps!
	);

	// On some cards glTextureSubImage2D crashes when data is nullptr
	if (data != nullptr) {
		GLCall(glTextureSubImage2D(mTextureID, 0,
			0, 0,
			width, height,
			(GLenum)translate(mData.colorspace),
			(GLenum)translate(mData.pixelDataType),
			data));
	}

	//GLCall(glTexImage2D((GLenum)mTarget, 0, (GLenum)translate(mData.internalFormat), width, height,
	//	0, (GLenum)translate(mData.colorspace), (GLenum)translate(mData.pixelDataType),
	//	data));


	// fill the allocated mipmaps
	if (mData.generateMipMaps) generateMipMaps();
}

nex::Texture2DGL::Texture2DGL(GLuint texture, const TextureData& textureData, unsigned width, unsigned height)
	: Impl(texture, TextureTargetGl::TEXTURE2D), mWidth(width), mHeight(height), mData(textureData)
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

	resizeTexImage2D(mTextureID, 1, mWidth, mHeight, (GLenum)translate(mData.internalFormat), mData.generateMipMaps);
}


nex::Texture2DMultisample::Texture2DMultisample(std::unique_ptr<Impl> impl) : Texture2D(std::move(impl))
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
	assert(false); //TODO
	return;
	mTarget = TextureTargetGl::TEXTURE2D_MULTISAMPLE;
	Impl::generateTexture(&mTextureID, textureData, (GLenum)mTarget);
	GLCall(glTexImage2DMultisample((GLenum)mTarget, mSamples, (GLenum)translate(mData.internalFormat), mWidth, mHeight, GL_TRUE));

	if (mData.generateMipMaps)
	{
		GLCall(glGenerateMipmap((GLenum)mTarget));
	}
}

nex::Texture2DMultisampleGL::Texture2DMultisampleGL(GLuint texture, const TextureData& textureData, unsigned samples,
	unsigned width, unsigned height) : Texture2DGL(texture, textureData, width, height), mSamples(samples)
{
	mTarget = TextureTargetGl::TEXTURE2D_MULTISAMPLE;
}

void nex::Texture2DMultisampleGL::resize(unsigned width, unsigned height)
{
	assert(false); //TODO
	return;
	mWidth = width;
	mHeight = height;
	GLCall(glTexImage2DMultisample((GLenum)mTarget, mSamples, (GLenum)translate(mData.internalFormat), mWidth, mHeight, GL_TRUE));

	if (mData.generateMipMaps)
	{
		GLCall(glGenerateMipmap((GLenum)mTarget));
	}
}

unsigned nex::Texture2DMultisampleGL::getSamples() const
{
	return mSamples;
}

nex::Texture2DArray::Texture2DArray(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::Texture2DArray::Texture2DArray(unsigned width, unsigned height, unsigned size, const TextureData& textureData, const void* data) :
	Texture(make_unique<Texture2DArrayGL>(width, height, size, textureData, data))
{
}

void nex::Texture2DArray::resize(unsigned width, unsigned height, unsigned depth)
{
	auto impl = (Texture2DArrayGL*)mImpl.get();
	impl->resize(width, height, depth);
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

unsigned nex::Texture2DArray::getDepth() const
{
	auto impl = (Texture2DArrayGL*)mImpl.get();
	return impl->getDepth();
}


nex::Texture2DArrayGL::Texture2DArrayGL(GLuint width, GLuint height, GLuint depth, const TextureData& textureData, const void* data) :
	Impl(GL_FALSE, TextureTargetGl::TEXTURE2D_ARRAY), mWidth(width), mHeight(height), mDepth(depth), mData(textureData)
{
	Impl::generateTexture(&mTextureID, textureData, (GLenum)mTarget);

		resizeTexImage3D(mTextureID,
			1,
			mWidth,
			mHeight,
			mDepth,
			(GLenum)translate(mData.internalFormat),
			mData.generateMipMaps);


		// set base mip map
		if (data != nullptr) {
			GLCall(glTextureSubImage3D(mTextureID, 0,
				0, 0, 0,
				width, height, depth,
				(GLenum)translate(mData.colorspace),
				(GLenum)translate(mData.pixelDataType),
				data));
		}
		

	// create content of the other mipmaps
	if (mData.generateMipMaps) generateMipMaps();

}

nex::Texture2DArrayGL::Texture2DArrayGL(GLuint texture, const TextureData& textureData, unsigned width, unsigned height, unsigned depth)
	: Impl(texture, TextureTargetGl::TEXTURE2D_ARRAY), mWidth(width), mHeight(height), mDepth(depth), mData(textureData)
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

unsigned nex::Texture2DArrayGL::getDepth() const
{
	return mDepth;
}

void nex::Texture2DArrayGL::setHeight(unsigned height)
{
	mHeight = height;
}

void nex::Texture2DArrayGL::setWidth(unsigned width)
{
	mWidth = width;
}

void nex::Texture2DArrayGL::setDepth(unsigned depth)
{
	mDepth = depth;
}

void nex::Texture2DArrayGL::resize(unsigned width, unsigned height, unsigned depth)
{
	///TODO
	assert(false);
	return;
	mWidth = width;
	mHeight = height;
	mDepth = depth;

	resizeTexImage3D(mTextureID, 
		1, 
		mWidth, 
		mHeight, 
		mDepth, 
		(GLenum)translate(mData.internalFormat),
		mData.generateMipMaps);
}

nex::TextureAccessGL nex::translate(nex::TextureAccess accessType)
{
	static TextureAccessGL const table[]
	{
		TextureAccessGL::READ_ONLY,
		TextureAccessGL::READ_WRITE,
		TextureAccessGL::WRITE_ONLY,
	};

	static const unsigned size = (unsigned)TextureAccess::LAST - (unsigned)TextureAccess::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureAccess and TextureAccessGL don't match!");

	return table[(unsigned)accessType];
}

nex::ChannelGL nex::translate(nex::Channel channel)
{
	static ChannelGL const table[]
	{
		ChannelGL::RED,
		ChannelGL::GREEN,
		ChannelGL::BLUE,
		ChannelGL::ALPHA,
	};

	static const unsigned size = (unsigned)Channel::LAST - (unsigned)Channel::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: Channel and ChannelGL don't match!");

	return table[(unsigned)channel];
}

nex::TextureFilterGL nex::translate(nex::TextureFilter filter)
{
	static TextureFilterGL const table[]
	{
		TextureFilterGL::NearestNeighbor,
		TextureFilterGL::Linear,
		TextureFilterGL::Near_Mipmap_Near,
		TextureFilterGL::Near_Mipmap_Linear,
		TextureFilterGL::Linear_Mipmap_Near,
		TextureFilterGL::Linear_Mipmap_Linear,
	};

	static const unsigned size = (unsigned)TextureFilter::LAST - (unsigned)TextureFilter::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureFilter and TextureFilterGL don't match!");

	return table[(unsigned)filter];
}

nex::TextureUVTechniqueGL nex::translate(nex::TextureUVTechnique technique)
{
	static TextureUVTechniqueGL const table[]
	{
		TextureUVTechniqueGL::ClampToBorder,
		TextureUVTechniqueGL::ClampToEdge,
		TextureUVTechniqueGL::MirrorRepeat,
		TextureUVTechniqueGL::MirrorClampToEdge,
		TextureUVTechniqueGL::Repeat,
	};

	static const unsigned size = (unsigned)TextureUVTechnique::LAST - (unsigned)TextureUVTechnique::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureUVTechnique and TextureUVTechniqueGL don't match!");

	return table[(unsigned)technique];
}

nex::ColorSpaceGL nex::translate(nex::ColorSpace colorSpace)
{
	static ColorSpaceGL const table[]
	{
		ColorSpaceGL::R,
		ColorSpaceGL::RED_INTEGER,
		ColorSpaceGL::RG,
		ColorSpaceGL::RG_INTEGER,
		ColorSpaceGL::RGB,
		ColorSpaceGL::RGBA,

		// srgb formats
		ColorSpaceGL::SRGB,
		ColorSpaceGL::SRGBA,

		ColorSpaceGL::DEPTH,
		ColorSpaceGL::STENCIL,
		ColorSpaceGL::DEPTH_STENCIL,
	};

	static const unsigned size = (unsigned)ColorSpace::LAST - (unsigned)ColorSpace::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: ColorSpace and ColorSpaceGL don't match!");

	return table[(unsigned)colorSpace];
}

nex::InternFormatGL nex::translate(nex::InternFormat format)
{
	static InternFormatGL const table[]
	{
		InternFormatGL::R8,
		InternFormatGL::R8UI,
		InternFormatGL::R16,
		InternFormatGL::R16F,
		InternFormatGL::R32F,
		InternFormatGL::R32I,
		InternFormatGL::R32UI,

		InternFormatGL::RG8,
		InternFormatGL::RG8UI,
		InternFormatGL::RG8_SNORM,
		InternFormatGL::RG16,
		InternFormatGL::RG16F,
		InternFormatGL::RG32F,
		InternFormatGL::RG32I,
		InternFormatGL::RG32UI,

		InternFormatGL::RGB8,
		InternFormatGL::RGB16,
		InternFormatGL::RGB16F,
		InternFormatGL::RGB32F,
		InternFormatGL::RGB32I,
		InternFormatGL::RGB32UI,

		InternFormatGL::RGBA8,
		InternFormatGL::RGBA16,
		InternFormatGL::RGBA16F,
		InternFormatGL::RGBA16_SNORM,
		InternFormatGL::RGBA32F,
		InternFormatGL::RGBA32I,
		InternFormatGL::RGBA32UI,

		// srgb formats
		InternFormatGL::SRGB8,
		InternFormatGL::SRGBA8,

		InternFormatGL::DEPTH24_STENCIL8,
		InternFormatGL::DEPTH32F_STENCIL8,
		InternFormatGL::DEPTH16,
		InternFormatGL::DEPTH24,
		InternFormatGL::DEPTH32,
		InternFormatGL::DEPTH_COMPONENT32F,
		InternFormatGL::STENCIL8,
	};

	static const unsigned size = (unsigned)InternFormat::LAST - (unsigned)InternFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: InternFormat and InternFormatGL don't match!");

	return table[(unsigned)format];
}

nex::PixelDataTypeGL nex::translate(nex::PixelDataType dataType)
{
	static PixelDataTypeGL const table[]
	{
		PixelDataTypeGL::FLOAT,
		PixelDataTypeGL::UBYTE,
		PixelDataTypeGL::UINT,
		PixelDataTypeGL::SHORT,

		PixelDataTypeGL::UNSIGNED_INT_24_8,
		PixelDataTypeGL::FLOAT_32_UNSIGNED_INT_24_8_REV,
		PixelDataTypeGL::UNSIGNED_SHORT,
		PixelDataTypeGL::UNSIGNED_INT_24,
		PixelDataTypeGL::UNSIGNED_INT_8,
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
		TextureTargetGl::TEXTURE1D,
		TextureTargetGl::TEXTURE1D_ARRAY,

		//2D
		TextureTargetGl::TEXTURE2D,
		TextureTargetGl::TEXTURE2D_MULTISAMPLE,

		// 3D
		TextureTargetGl::TEXTURE2D_ARRAY,
		TextureTargetGl::TEXTURE2D_MULTISAMPLE_ARRAY,
		TextureTargetGl::TEXTURE3D,

		// cubemap
		TextureTargetGl::CUBE_MAP,
	};

	static const unsigned size = (unsigned)TextureTarget::LAST - (unsigned)TextureTarget::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureTarget and TextureTargetGl don't match!");

	return table[(unsigned)target];
}