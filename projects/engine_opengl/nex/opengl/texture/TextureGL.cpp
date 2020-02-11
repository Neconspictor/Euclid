#include "..\..\..\..\engine\nex\texture\TextureSamplerData.hpp"
#include <nex/opengl/texture/TextureGL.hpp>
#include <cassert>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Image.hpp>
#include <nex/texture/Texture.hpp>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/opengl/opengl.hpp>
#include <nex/opengl/RenderBackendGL.hpp>
#include "nex/opengl/CacheGL.hpp"
#include <nex/texture/Sampler.hpp>
#include <nex/opengl/texture/SamplerGL.hpp>
#include <nex/util/Memory.hpp>

using namespace std;
using namespace glm;

std::vector<glm::mat4> nex::CubeMap::mViewLookAts = {
	lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)), // right
	lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)), // left
	lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)), // top
	lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)), // bottom
	lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)), // front
	lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)) // back
};

const mat4& nex::CubeMap::getViewLookAtMatrix(CubeMapSide side)
{
	return mViewLookAts[(unsigned)side];
}

const std::vector<glm::mat4>& nex::CubeMap::getViewLookAts()
{
	return mViewLookAts;
}

nex::CubeMap::CubeMap(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::CubeMap::CubeMap(unsigned sideWidth, unsigned sideHeight, const TextureDesc& data) : 
	Texture(make_unique<CubeMapGL>(sideWidth, sideHeight, data, nullptr))
{
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

nex::CubeMapGL::CubeMapGL(unsigned sideWidth, unsigned sideHeight, const TextureDesc& data, const TextureTransferDesc* desc) : 
	Impl(TextureTarget::CUBE_MAP, data, sideWidth, sideHeight, 6, 1)
{
	const auto internalFormat = (GLenum)nex::translate(data.internalFormat);

	generateTexture(&mTextureID, data, (GLenum)mTargetGL);
	resizeTexImage2D(mTextureID, 1, mWidth, mHeight, internalFormat, data.generateMipMaps);

	if (desc) upload(*desc);

	if (data.generateMipMaps) generateMipMaps();
	else updateMipMapCount();
}

nex::CubeMapGL::CubeMapGL(GLuint cubeMap, unsigned sideWidth, unsigned sideHeight, const TextureDesc& data) : 
Impl(cubeMap, TextureTarget::CUBE_MAP, data, sideWidth, sideHeight, 6, 1)
{
	updateMipMapCount();
}

GLuint nex::CubeMapGL::getCubeMap() const
{
	return mTextureID;
}

void nex::CubeMapGL::setCubeMap(GLuint id)
{
	mTextureID = id;
}

void nex::CubeMapGL::upload(const TextureTransferDesc& desc)
{
	// On some cards glTextureSubImage2D crashes when data is nullptr
	if (!desc.data) return;

	const auto& imageDesc = desc.imageDesc;

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, imageDesc.rowByteAlignmnet));

	GLCall(glTextureSubImage3D(mTextureID,
		desc.mipMapLevel,
		desc.xOffset, desc.yOffset, desc.zOffset,
		imageDesc.width, imageDesc.height, imageDesc.depth,

		(GLenum)nex::translate(imageDesc.colorspace),
		(GLenum)nex::translate(imageDesc.pixelDataType),
		desc.data));

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, PIXEL_STORE_DEFAULT_ALIGNMENT));
}


nex::ColorSpace nex::getColorSpace(InternalFormat format)
{
	static ColorSpace const table[]
	{
		ColorSpace::R,
		ColorSpace::RED_INTEGER,
		ColorSpace::R,
		ColorSpace::R,
		ColorSpace::R,
		ColorSpace::RED_INTEGER,
		ColorSpace::RED_INTEGER,

		ColorSpace::RG,
		ColorSpace::RG_INTEGER,
		ColorSpace::RG_INTEGER,
		ColorSpace::RG,
		ColorSpace::RG,
		ColorSpace::RG,
		ColorSpace::RG_INTEGER,
		ColorSpace::RG_INTEGER,

		ColorSpace::RGB,
		ColorSpace::RGB,
		ColorSpace::RGB,
		ColorSpace::RGB,
		ColorSpace::RGB,
		ColorSpace::RGB,
		ColorSpace::RGB,

		ColorSpace::RGBA,
		ColorSpace::RGBA,
		ColorSpace::RGBA,
		ColorSpace::RGBA,
		ColorSpace::RGBA,
		ColorSpace::RGBA,
		ColorSpace::RGBA,

		ColorSpace::RGBA,
		ColorSpace::RGBA,

		ColorSpace::RGB,
		ColorSpace::RGBA,

		ColorSpace::DEPTH_STENCIL,
		ColorSpace::DEPTH_STENCIL,
		ColorSpace::DEPTH,
		ColorSpace::DEPTH,
		ColorSpace::DEPTH,
		ColorSpace::DEPTH,
		ColorSpace::STENCIL
	};

	static const unsigned size = (unsigned)InternalFormat::LAST - (unsigned)InternalFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: internal format to colorspace map doesn't match number of supported internal formats");

	return table[(unsigned)format];
}

unsigned nex::getComponents(InternalFormat format)
{
	static unsigned const table[]
	{
		1,
		1,
		1,
		1,
		1,
		1,
		1,

		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2,

		3,
		3,
		3,
		3,
		3,
		3,
		3,

		4,
		4,
		4,
		4,
		4,
		4,
		4,

		4,
		4,

		3,
		4,

		2,
		2,
		1,
		1,
		1,
		1,
		1
	};

	static const unsigned size = (unsigned)InternalFormat::LAST - (unsigned)InternalFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: internal format to pixel data type map doesn't match number of supported internal formats");

	return table[(unsigned)format];
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
		3,
		3,
		3,

		4,
		4,
		4,
		4,

		3,
		4,

		1,
		1,
		2,
	};

	static const unsigned size = (unsigned)ColorSpace::LAST - (unsigned)ColorSpace::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)colorSpace];
}

unsigned nex::getPixelDataTypeByteSize(const PixelDataType pixelDataType)
{
	static unsigned const table[]
	{
		4,
		2,
		4,
		1,
		4,

		2,
		
		4,
		8,
		2,
		4,
		4,

		4


	};

	static const unsigned size = (unsigned)PixelDataType::LAST - (unsigned)PixelDataType::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: pixel data type descriptor list doesn't match number of supported pixel data types");

	return table[(unsigned)pixelDataType];
}

unsigned nex::getPixelDataTypePackedComponentsCount(const PixelDataType pixelDataType)
{
	static unsigned const table[]
	{
		1, 
		1, 
		1,
		1,
		1,

		1,

		2,
		2,
		1,
		1,
		4,

		4
	};

	static const unsigned size = (unsigned)PixelDataType::LAST - (unsigned)PixelDataType::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: pixel data type descriptor list doesn't match number of supported pixel data types");

	return table[(unsigned)pixelDataType];
}

nex::InternalFormatType nex::getType(InternalFormat format) {
	static InternalFormatType const table[]
	{
		InternalFormatType::NORMAL,
		InternalFormatType::UINT,
		InternalFormatType::NORMAL,
		InternalFormatType::FLOAT,
		InternalFormatType::FLOAT,
		InternalFormatType::INT,
		InternalFormatType::UINT,

		InternalFormatType::NORMAL,
		InternalFormatType::UINT,
		InternalFormatType::SNORM,
		InternalFormatType::NORMAL,
		InternalFormatType::FLOAT,
		InternalFormatType::FLOAT,
		InternalFormatType::INT,
		InternalFormatType::UINT,

		InternalFormatType::NORMAL,
		InternalFormatType::NORMAL,
		InternalFormatType::NORMAL,
		InternalFormatType::FLOAT,
		InternalFormatType::FLOAT,
		InternalFormatType::INT,
		InternalFormatType::UINT,

		InternalFormatType::NORMAL,
		InternalFormatType::NORMAL,
		InternalFormatType::FLOAT,
		InternalFormatType::SNORM,
		InternalFormatType::FLOAT,
		InternalFormatType::INT,
		InternalFormatType::UINT,

		InternalFormatType::NORMAL,
		InternalFormatType::UINT,

		InternalFormatType::NORMAL,
		InternalFormatType::NORMAL,

		InternalFormatType::COMBINED,
		InternalFormatType::COMBINED,
		InternalFormatType::NORMAL,
		InternalFormatType::NORMAL,
		InternalFormatType::NORMAL,
		InternalFormatType::FLOAT,
		InternalFormatType::NORMAL
	};

	static const unsigned size = (unsigned)InternalFormat::LAST - (unsigned)InternalFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: internal format type map doesn't match number of supported internal formats");

	return table[(unsigned)format];

}

bool nex::isSRGB(InternalFormat format) {
	return format == InternalFormat::SRGB8
		|| format == InternalFormat::SRGBA8;
}

nex::RenderBuffer::RenderBuffer(unsigned width, unsigned height, int samples, const TextureDesc& data) : 
	Texture(make_unique<RenderBufferGL>(width, height, samples, data))
{
}

nex::InternalFormat nex::RenderBuffer::getFormat() const
{
	auto gl = (RenderBufferGL*)mImpl.get();
	return gl->getFormat();
}


nex::RenderBufferGL::RenderBufferGL(GLuint width, GLuint height, int samples, const TextureDesc& data) :
Impl(GL_FALSE, TextureTarget::RENDER_BUFFER, data, width, height, 1, samples)
{
	GLCall(glGenRenderbuffers(1, &mTextureID));
	GLCall(glBindRenderbuffer((GLenum)mTargetGL, mTextureID));
	resize(mWidth, mHeight);
}

nex::RenderBufferGL::~RenderBufferGL()
{
	if (mTextureID != GL_FALSE) {
		GLCall(glDeleteRenderbuffers(1, &mTextureID));
		mTextureID = GL_FALSE;
	}
}

nex::RenderBufferGL::RenderBufferGL(GLuint texture, GLuint width, GLuint height, int samples, const TextureDesc& data)
	: Impl(texture, TextureTarget::RENDER_BUFFER, data, width, height, 1, samples)
{
	updateMipMapCount();
}

nex::InternalFormat nex::RenderBufferGL::getFormat() const
{
	return mTextureData.internalFormat;
}

void nex::RenderBufferGL::resize(unsigned width, unsigned height)
{
	mWidth = width;
	mHeight = height;

	if (mSamples > 1) {
		GLCall(glNamedRenderbufferStorageMultisample(mTextureID, mSamples, (GLenum)translate(mTextureData.internalFormat), width, height));
	}
	else {
		GLCall(glNamedRenderbufferStorage(mTextureID, (GLenum)translate(mTextureData.internalFormat), width, height));
	}
	updateMipMapCount();
}

void nex::RenderBufferGL::upload(const TextureTransferDesc& desc)
{
	//Not supported yet
}

nex::Texture::~Texture() = default;

nex::Texture::Texture(std::unique_ptr<Impl> impl) : mImpl(std::move(impl))
{
}

nex::Texture* nex::Texture::createFromImage(const StoreImage& store, const TextureDesc& d)
{
	TextureDesc data = d;
	data.minLOD = 0;
	data.maxLOD = store.mipmapCount - 1;
	data.lodBaseLevel = 0;
	data.lodMaxLevel = store.mipmapCount - 1;

	const auto& baseImageDesc = store.images[0][0].desc;

	const auto format = (GLenum)translate(baseImageDesc.colorspace);
	const auto internalFormat = (GLenum)translate(data.internalFormat);
	const auto pixelDataType = (GLenum)translate(baseImageDesc.pixelDataType);
	const auto bindTarget = (GLenum)translate(store.textureTarget);

	// For now only cubemaps and 2d tetures are supported (other targets are not tested yet)!
	assert(store.textureTarget == TextureTarget::TEXTURE2D || store.textureTarget == TextureTarget::CUBE_MAP);
	const bool isCubeMap = store.textureTarget == TextureTarget::CUBE_MAP;

	const bool createMipMapStorage = store.mipmapCount > 1;

	GLuint textureID;
	Impl::generateTexture(&textureID, data, bindTarget);

	// allocate texture storage
	// Note: for cubemaps six sides are allocated automatically!
	Impl::resizeTexImage2D(textureID, store.mipmapCount, baseImageDesc.width, baseImageDesc.height, internalFormat, createMipMapStorage);

	for (unsigned int side = 0; side < store.images.size(); ++side)
	{
		for (unsigned mipMapLevel = 0; mipMapLevel < store.images[side].size(); ++mipMapLevel)
		{
			const auto& image = store.images[side][mipMapLevel];
			const auto& desc = image.desc;

			if (isCubeMap)
			{
				GLCall(glTextureSubImage3D(textureID,
					mipMapLevel,
					0, 0,
					side, // zoffset specifies the cubemap side
					desc.width, desc.height,
					1, // depth specifies the number of sides to be updated
					format,
					pixelDataType,
					image.pixels.getPixels()));
			} else
			{
				GLCall(glTextureSubImage2D(textureID,
					mipMapLevel,
					0, 0,
					desc.width, desc.height,
					format,
					pixelDataType,
					image.pixels.getPixels()));
			}
		}
	}

	std::unique_ptr<Impl> impl;
	std::unique_ptr<Texture> result;

	if (isCubeMap)
	{
		impl = std::make_unique<CubeMapGL>(textureID, baseImageDesc.width, baseImageDesc.height, data);
		result = std::make_unique<CubeMap>(std::move(impl));
	} else
	{
		impl = std::make_unique<Texture2DGL>(textureID, data, baseImageDesc.width, baseImageDesc.height);
		result = std::make_unique<Texture2D>(std::move(impl));
	}

	if (data.generateMipMaps)
	{
		result->generateMipMaps();
	}


	return result.release();
}

std::unique_ptr<nex::Texture> nex::Texture::createView(Texture* original, TextureTarget target, unsigned startLevel, unsigned numLevel,
	unsigned startLayer, unsigned numLayers, const TextureDesc& data)
{
	auto impl = Impl::createView(original->getImpl(), target, startLevel, numLevel, startLayer, numLayers, data);
	return make_unique<Texture>(std::move(impl));
}

void nex::Texture::generateMipMaps()
{
	mImpl->generateMipMaps();
}

nex::TextureTarget nex::Texture::getTarget() const
{
	return mImpl->getTarget();
}

unsigned nex::Texture::getMipMapCount() const
{
	return mImpl->getMipMapCount();
}

bool nex::Texture::hasNonBaseLevelMipMaps() const
{
	return mImpl->hasNonBaseLevelMipMaps();
}

const nex::TextureDesc& nex::Texture::getTextureData() const
{
	return ((Texture::Impl*)getImpl())->getTextureData();
}

unsigned nex::Texture::getWidth() const
{
	return mImpl->getWidth();
}

unsigned nex::Texture::getHeight() const
{
	return mImpl->getHeight();
}

unsigned nex::Texture::getDepth() const
{
	return mImpl->getDepth();
}

const glm::uvec2& nex::Texture::getTileCount() const {
	return mImpl->getTileCount();
}

void nex::Texture::setTileCount(glm::uvec2 tileCount) {
	mImpl->setTileCount(std::move(tileCount));
}


nex::Texture::Impl* nex::Texture::getImpl() const
{
	return mImpl.get();
}

unsigned nex::Texture::getLevelZeroMipMapTextureSize()
{
	return std::max<unsigned>(getWidth(), getHeight());
}

unsigned nex::Texture::calcMipMapCount(unsigned levelZeroMipMapTextureSize)
{
	return std::log2<>(levelZeroMipMapTextureSize) + 1;
}

void nex::Texture::readback(TextureTransferDesc& desc) const
{
	const auto& imageDesc = desc.imageDesc;
	RenderBackend::get()->syncMemoryWithGPU(MemorySync_TextureUpdate | MemorySync_ShaderImageAccess);
	GLCall(glGetTextureSubImage(mImpl->mTextureID,
		desc.mipMapLevel,
		desc.xOffset, desc.yOffset, desc.zOffset,
		imageDesc.width, imageDesc.height, imageDesc.depth,
		(GLenum)translate(imageDesc.colorspace),
		(GLenum)translate(imageDesc.pixelDataType),
		desc.dataByteSize,
		desc.data
	));
}

void nex::Texture::setImpl(std::unique_ptr<Impl> impl)
{
	mImpl = std::move(impl);
}

uint64_t nex::Texture::getHandle()
{
	uint64_t handle;
	GLCall(handle = glGetTextureHandleARB(*mImpl->getTexture()));
	return handle;
}

uint64_t nex::Texture::getHandleWithSampler(const nex::Sampler & sampler)
{
	uint64_t handle;
	GLCall(handle = glGetTextureSamplerHandleARB(*mImpl->getTexture(), sampler.getImpl()->getID()));
	return handle;
}

void nex::Texture::residentHandle(uint64_t handle)
{
	GLCall(glMakeTextureHandleResidentARB(handle));
}

void nex::Texture::makeHandleNonResident(uint64_t handle)
{
	GLCall(glMakeTextureHandleNonResidentARB(handle));
}


void nex::Texture::upload(const TextureTransferDesc& desc)
{
	mImpl->upload(desc);
}

nex::Texture::Impl::Impl(TextureTarget target, const TextureDesc& data, unsigned width, unsigned height, unsigned depth, unsigned samples) : 
	Impl(GL_FALSE, target, data, width, height, depth, samples)
{
}

nex::Texture::Impl::Impl(GLuint texture, TextureTarget target, const TextureDesc& data, unsigned width, unsigned height, unsigned depth, unsigned samples) : 
mTextureID(texture), mTarget(target),
mTargetGL(translate(target)), mTextureData(data),
mWidth(width), mHeight(height), mDepth(depth),
mTileCount(1,1),
mSamples(samples)
{
}


nex::Texture::Impl::~Impl()
{
	release();
}

const nex::TextureDesc& nex::Texture::Impl::getTextureData() const
{
	return mTextureData;
}

void nex::Texture::Impl::generateMipMaps()
{
	//GLCall(glTextureParameteri(mTextureID, GL_TEXTURE_BASE_LEVEL, 0));
	//GLCall(glTextureParameteri(mTextureID, GL_TEXTURE_MAX_LEVEL, 1000));
	GLCall(glGenerateTextureMipmap(mTextureID));

	updateMipMapCount();
}

std::unique_ptr<nex::Texture::Impl> nex::Texture::Impl::createView(Impl* original, TextureTarget target, unsigned startLevel, unsigned numLevel,
	unsigned startLayer, unsigned numLayers, const TextureDesc& data)
{
	auto result = make_unique<Impl>(target, data, original->getWidth(), original->getHeight(), original->getDepth(), original->getSamples());
	// TODO check whether target and the target of the original texture are compatible!
	const auto targetGL = (GLenum)translate(target);

	const auto internalFormat = (GLenum)translate(data.internalFormat);


	// Note: we mustn't bind the texture to a target (necessary for glTextureView)
	GLCall(glGenTextures(1, result->getTexture()));
	const GLuint textureID = *result->getTexture();

	GLCall(glTextureView(textureID, targetGL, *original->getTexture(), internalFormat, startLevel, numLevel, startLayer, numLayers));
	applyTextureData(textureID, result->getSamples() > 1, data);
	return result;
}

void nex::Texture::Impl::generateTexture(GLuint* out, const BaseTextureDesc& desc, GLenum target)
{
	GLCall(glCreateTextures(target, 1, out));

	bool isMultisample = target == GL_TEXTURE_2D_MULTISAMPLE || target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
		applyTextureData(*out, isMultisample, desc);
}

void nex::Texture::Impl::applyTextureData(GLuint texture, bool isMultisample, const BaseTextureDesc& desc)
{
	if (isMultisample) return;

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
	GLCall(glTextureParameteri(texture, GL_DEPTH_STENCIL_TEXTURE_MODE, (GLenum)translate(desc.depthStencilTextureMode)));


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
		GLCall(glTextureParameteri(texture, GL_TEXTURE_COMPARE_FUNC, (GLenum)translate(desc.compareFunction)));
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

void nex::Texture::Impl::resizeTexImage1D(GLuint textureID, GLint levels, unsigned width, GLint internalFormat, bool generateMipMaps)
{
	if (generateMipMaps)
	{
		levels = std::log2<>(width) + 1;
	}

	GLCall(glTextureStorage1D(textureID, levels, internalFormat, width));
	GLCall(glTextureParameteri(textureID, GL_TEXTURE_BASE_LEVEL, 0));
	GLCall(glTextureParameteri(textureID, GL_TEXTURE_MAX_LEVEL, levels - 1));
}

void nex::Texture::Impl::resizeTexImage2D(GLuint textureID, GLint levels, unsigned width, unsigned height,
	GLint internalFormat, bool generateMipMaps)
{
	if (generateMipMaps)
	{
		levels = std::log2<>(std::min<>(width, height)) + 1;
	}

	GLCall(glTextureStorage2D(textureID, levels, internalFormat, width, height));
	GLCall(glTextureParameteri(textureID, GL_TEXTURE_BASE_LEVEL, 0));
	GLCall(glTextureParameteri(textureID, GL_TEXTURE_MAX_LEVEL, levels - 1));
}

void nex::Texture::Impl::resizeTexImage3D(GLuint textureID, GLint levels, unsigned width, unsigned height,
	unsigned depth, GLint internalFormat, bool generateMipMaps)
{

	if (generateMipMaps)
	{
		levels = std::log2<>(std::min<>(width, height)) + 1;
	}

	GLCall(glTextureStorage3D(textureID, levels, internalFormat, width, height, depth));
	GLCall(glTextureParameteri(textureID, GL_TEXTURE_BASE_LEVEL, 0));
	GLCall(glTextureParameteri(textureID, GL_TEXTURE_MAX_LEVEL, levels - 1));
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

nex::TextureTarget nex::Texture::Impl::getTarget() const
{
	return mTarget;
}

nex::TextureTargetGl nex::Texture::Impl::getTargetGL() const
{
	return mTargetGL;
}

unsigned nex::Texture::Impl::getWidth() const
{
	return mWidth;
}

unsigned nex::Texture::Impl::getHeight() const
{
	return mHeight;
}

unsigned nex::Texture::Impl::getDepth() const
{
	return mDepth;
}

unsigned nex::Texture::Impl::getSamples() const
{
	return mSamples;
}

const glm::uvec2& nex::Texture::Impl::getTileCount() const
{
	return mTileCount;
}

bool nex::Texture::Impl::hasNonBaseLevelMipMaps() const
{
	return getMipMapCount() > 1;
}

unsigned nex::Texture::Impl::getMipMapCount() const
{
	return mTextureData.lodMaxLevel - mTextureData.lodBaseLevel + 1;
}

void nex::Texture::Impl::setHeight(unsigned height)
{
	mHeight = height;
}

void nex::Texture::Impl::setWidth(unsigned width)
{
	mWidth = width;
}

void nex::Texture::Impl::setDepth(unsigned depth)
{
	mDepth = depth;
}

void nex::Texture::Impl::setTileCount(glm::uvec2 tileCount)
{
	mTileCount = std::move(tileCount);
}

void nex::Texture::Impl::updateMipMapCount()
{
	GLCall(glGetTextureParameteriv(mTextureID, GL_TEXTURE_BASE_LEVEL, (int*)&mTextureData.lodBaseLevel));
	GLCall(glGetTextureParameteriv(mTextureID, GL_TEXTURE_MAX_LEVEL, (int*)&mTextureData.lodMaxLevel));
	GLCall(glGetTextureParameteriv(mTextureID, GL_TEXTURE_BASE_LEVEL, &mTextureData.minLOD));
	GLCall(glGetTextureParameteriv(mTextureID, GL_TEXTURE_MAX_LEVEL, &mTextureData.maxLOD));

}

void nex::Texture::Impl::upload(const TextureTransferDesc& desc)
{
}


nex::Texture1DGL::Texture1DGL(GLuint width, const TextureDesc& textureData, const TextureTransferDesc* desc) :
	Impl(GL_FALSE, TextureTarget::TEXTURE1D, textureData, width, 1, 1, 1)
{
	generateTexture(&mTextureID, textureData, (GLenum)mTargetGL);


	resizeTexImage1D(mTextureID,
		1, // levels will be calculated automatically when generateMipMaps is true 
		width,
		(GLenum)translate(mTextureData.internalFormat),
		mTextureData.generateMipMaps // specifies the storage for mipmaps!
	);

	if (desc) upload(*desc);

	// fill the allocated mipmaps
	if (mTextureData.generateMipMaps) generateMipMaps();
	else updateMipMapCount();
}

nex::Texture1DGL::Texture1DGL(GLuint texture, const TextureDesc& textureData, unsigned width)
	: Impl(texture, TextureTarget::TEXTURE1D, textureData, width, 1, 1, 1)
{
	updateMipMapCount();
}

void nex::Texture1DGL::resize(unsigned width, unsigned mipmapCount, bool autoMipMapCount)
{
	mWidth = width;
	mTextureData.generateMipMaps = autoMipMapCount;

	resizeTexImage1D(mTextureID, mipmapCount, mWidth, (GLenum)translate(mTextureData.internalFormat), autoMipMapCount);
	updateMipMapCount();
}

void nex::Texture1DGL::upload(const TextureTransferDesc& desc)
{
	// On some cards glTextureSubImage2D crashes when data is nullptr
	if (!desc.data) return;

	const auto& imageDesc = desc.imageDesc;

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, imageDesc.rowByteAlignmnet));

	GLCall(glTextureSubImage1D(mTextureID, desc.mipMapLevel,
		desc.xOffset,
		imageDesc.width,
		(GLenum)translate(imageDesc.colorspace),
		(GLenum)translate(imageDesc.pixelDataType),
		desc.data));

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, PIXEL_STORE_DEFAULT_ALIGNMENT));
}

nex::Texture1D::Texture1D(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::Texture1D::Texture1D(unsigned width, const TextureDesc& textureData, const TextureTransferDesc* data)
	:
	Texture(make_unique<Texture1DGL>(width, textureData, data))
{
}

void nex::Texture1D::resize(unsigned width, unsigned mipmapCount, bool autoMipMapCount)
{
	Texture1DGL* impl = (Texture1DGL*)mImpl.get();
	impl->resize(width, mipmapCount, autoMipMapCount);
}

nex::Texture1DArray::Texture1DArray(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::Texture1DArray::Texture1DArray(unsigned width, unsigned height, const TextureDesc& textureData, const TextureTransferDesc* data)
	:
	Texture(make_unique<Texture1DArrayGL>(width, height, textureData, data))
{
}

void nex::Texture1DArray::resize(unsigned width, unsigned height, unsigned mipmapCount, bool autoMipMapCount)
{
	Texture1DArrayGL* impl = (Texture1DArrayGL*)mImpl.get();
	impl->resize(width, height, mipmapCount, autoMipMapCount);
}


nex::Texture1DArrayGL::Texture1DArrayGL(GLuint width, GLuint height, const TextureDesc& textureData, const TextureTransferDesc* desc)
	: Impl(GL_FALSE, TextureTarget::TEXTURE1D_ARRAY, textureData, width, height, 1, 1)
{
	generateTexture(&mTextureID, textureData, (GLenum)mTargetGL);

	resizeTexImage2D(mTextureID,
		1, // levels will be calculated automatically when generateMipMaps is true 
		width,
		height,
		(GLenum)translate(mTextureData.internalFormat),
		mTextureData.generateMipMaps // specifies the storage for mipmaps!
	);

	if (desc) upload(*desc);

	// fill the allocated mipmaps
	if (mTextureData.generateMipMaps) generateMipMaps();
	else updateMipMapCount();
}

nex::Texture1DArrayGL::Texture1DArrayGL(GLuint texture, const TextureDesc& textureData, unsigned width, unsigned height)
	: Impl(texture, TextureTarget::TEXTURE1D_ARRAY, textureData, width, height, 1, 1)
{
	updateMipMapCount();
}

void nex::Texture1DArrayGL::resize(unsigned width, unsigned height, unsigned mipmapCount, bool autoMipMapCount)
{
	mWidth = width;
	mHeight = height;
	mTextureData.generateMipMaps = autoMipMapCount;

	resizeTexImage2D(mTextureID, mipmapCount, mWidth, mHeight, (GLenum)translate(mTextureData.internalFormat), autoMipMapCount);
	updateMipMapCount();
}

void nex::Texture1DArrayGL::upload(const TextureTransferDesc& desc)
{
	// On some cards glTextureSubImage2D crashes when data is nullptr
	if (!desc.data) return;

	const auto& imageDesc = desc.imageDesc;

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, imageDesc.rowByteAlignmnet));

	GLCall(glTextureSubImage2D(mTextureID, desc.mipMapLevel,
		desc.xOffset, desc.yOffset,
		imageDesc.width, imageDesc.height,
		(GLenum)translate(imageDesc.colorspace),
		(GLenum)translate(imageDesc.pixelDataType),
		desc.data));

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, PIXEL_STORE_DEFAULT_ALIGNMENT));
}



nex::Texture2D::Texture2D(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::Texture2D::Texture2D(unsigned width, unsigned height, const TextureDesc& textureData, const TextureTransferDesc* data)
	:
	Texture(make_unique<Texture2DGL>(width, height, textureData, data))
{
}

void nex::Texture2D::resize(unsigned width, unsigned height, unsigned mipmapCount, bool autoMipMapCount)
{
	Texture2DGL* impl = (Texture2DGL*)mImpl.get();
	impl->resize(width, height, mipmapCount, autoMipMapCount);
}

// const void* data, width, height, alignment, pixel data type, color space

nex::Texture2DGL::Texture2DGL(GLuint width, GLuint height, const TextureDesc& textureData, const TextureTransferDesc* desc) :
	Impl(GL_FALSE, TextureTarget::TEXTURE2D, textureData, width, height, 1, 1)
{
	generateTexture(&mTextureID, textureData, (GLenum)mTargetGL);
	resizeTexImage2D(mTextureID,
		1, // levels will be calculated automatically when generateMipMaps is true
		mWidth, mHeight,
		(GLenum)translate(mTextureData.internalFormat),
		mTextureData.generateMipMaps // specifies the storage for mipmaps!
	);	
	
	if (desc) upload(*desc);

	// fill the allocated mipmaps
	if (mTextureData.generateMipMaps) generateMipMaps();
	else updateMipMapCount();
}

nex::Texture2DGL::Texture2DGL(GLuint texture, const TextureDesc& textureData, unsigned width, unsigned height)
	: Impl(texture, TextureTarget::TEXTURE2D, textureData, width, height, 1, 1)
{
	updateMipMapCount();
}

void nex::Texture2DGL::resize(unsigned width, unsigned height, unsigned mipmapCount, bool autoMipMapCount)
{
	mWidth = width;
	mHeight = height;
	mTextureData.generateMipMaps = autoMipMapCount;

	resizeTexImage2D(mTextureID, mipmapCount, mWidth, mHeight, (GLenum)translate(mTextureData.internalFormat), autoMipMapCount);
	updateMipMapCount();
}

void nex::Texture2DGL::upload(const TextureTransferDesc& desc)
{
	// On some cards glTextureSubImage2D crashes when data is nullptr
	if (!desc.data) return;

	const auto& imageDesc = desc.imageDesc;

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, imageDesc.rowByteAlignmnet));

	GLCall(glTextureSubImage2D(mTextureID, desc.mipMapLevel,
		desc.xOffset, desc.yOffset,
		imageDesc.width, imageDesc.height,
		(GLenum)translate(imageDesc.colorspace),
		(GLenum)translate(imageDesc.pixelDataType),
		desc.data));

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, PIXEL_STORE_DEFAULT_ALIGNMENT));
}


nex::Texture2DMultisample::Texture2DMultisample(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::Texture2DMultisample::Texture2DMultisample(unsigned width, unsigned height, const TextureDesc& textureData,
	unsigned samples) : Texture(make_unique<Texture2DMultisampleGL>(width, height, textureData, samples))
{
}

void nex::Texture2DMultisample::resize(unsigned width, unsigned height)
{
	auto impl = (Texture2DMultisampleGL*)mImpl.get();
	impl->resize(width, height);
	impl->updateMipMapCount();
}

unsigned nex::Texture2DMultisample::getSamples() const
{
	auto impl = (Texture2DMultisampleGL*)mImpl.get();
	return impl->getSamples();
}

nex::Texture2DMultisampleGL::Texture2DMultisampleGL(GLuint width, GLuint height, const TextureDesc& textureData,
	unsigned samples) : Impl(GL_FALSE, TextureTarget::TEXTURE2D_MULTISAMPLE, textureData, width, height, 1, samples)
{
	Impl::generateTexture(&mTextureID, textureData, (GLenum)mTargetGL);
	resize(width, height);

}

nex::Texture2DMultisampleGL::Texture2DMultisampleGL(GLuint texture, const TextureDesc& textureData, unsigned samples,
	unsigned width, unsigned height) : Impl(texture, TextureTarget::TEXTURE2D_MULTISAMPLE, textureData, width, height, 1, samples)
{
	mTargetGL = TextureTargetGl::TEXTURE2D_MULTISAMPLE;
}

void nex::Texture2DMultisampleGL::resize(unsigned width, unsigned height, unsigned mipmapCount, bool autoMipMapCount)
{
	mWidth = width;
	mHeight = height;

	GLCall(glTextureStorage2DMultisample(mTextureID, mSamples, (GLenum)translate(mTextureData.internalFormat), mWidth, mHeight, GL_TRUE));

	updateMipMapCount();
}

unsigned nex::Texture2DMultisampleGL::getSamples() const
{
	return mSamples;
}

void nex::Texture2DMultisampleGL::upload(const TextureTransferDesc& desc)
{
	//Not supported yet
}

nex::Texture2DArray::Texture2DArray(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::Texture2DArray::Texture2DArray(unsigned width, unsigned height, unsigned size, const TextureDesc& textureData, const TextureTransferDesc* data) :
	Texture(make_unique<Texture2DArrayGL>(width, height, size, textureData, data))
{
}

void nex::Texture2DArray::resize(unsigned width, unsigned height, unsigned depth, unsigned mipmapCount, bool autoMipMapCount)
{
	auto impl = (Texture2DArrayGL*)mImpl.get();
	impl->resize(width, height, depth, mipmapCount, autoMipMapCount);
}

nex::Texture2DArrayGL::Texture2DArrayGL(GLuint width, GLuint height, GLuint depth, const TextureDesc& textureData, const TextureTransferDesc* desc) :
	Impl(GL_FALSE, TextureTarget::TEXTURE2D_ARRAY, textureData, width, height, depth, 1)
{
	Impl::generateTexture(&mTextureID, textureData, (GLenum)mTargetGL);

	resize(mWidth, mHeight, mDepth, 1, mTextureData.generateMipMaps);


	if (desc) upload(*desc);
		
	// create content of the other mipmaps
	if (mTextureData.generateMipMaps) generateMipMaps();
	else updateMipMapCount();

}

nex::Texture2DArrayGL::Texture2DArrayGL(GLuint texture, const TextureDesc& textureData, unsigned width, unsigned height, unsigned depth)
	: Impl(texture, TextureTarget::TEXTURE2D_ARRAY, textureData, width, height, depth, 1)
{
	updateMipMapCount();
}

void nex::Texture2DArrayGL::resize(unsigned width, unsigned height, unsigned depth, unsigned mipmapCount, bool autoMipMapCount)
{
	mWidth = width;
	mHeight = height;
	mDepth = depth;
	mTextureData.generateMipMaps = autoMipMapCount;

	resizeTexImage3D(mTextureID, 
		mipmapCount,
		mWidth, 
		mHeight, 
		mDepth, 
		(GLenum)translate(mTextureData.internalFormat),
		autoMipMapCount);

	updateMipMapCount();
}

void nex::Texture2DArrayGL::upload(const TextureTransferDesc& desc)
{
	// On some cards glTextureSubImage2D crashes when data is nullptr
	if (!desc.data) return;

	const auto& imageDesc = desc.imageDesc;

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, imageDesc.rowByteAlignmnet));

	GLCall(glTextureSubImage3D(mTextureID, desc.mipMapLevel,
		desc.xOffset, desc.yOffset, desc.zOffset,
		imageDesc.width, imageDesc.height, imageDesc.depth,
		(GLenum)translate(imageDesc.colorspace),
		(GLenum)translate(imageDesc.pixelDataType),
		desc.data));

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, PIXEL_STORE_DEFAULT_ALIGNMENT));
}

nex::Texture3D::Texture3D(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::Texture3D::Texture3D(unsigned width, unsigned height, unsigned depth, const TextureDesc& textureData, const TextureTransferDesc* data) :
	Texture(make_unique<Texture3DGL>(width, height, depth, textureData, data))
{

}

void nex::Texture3D::resize(unsigned width, unsigned height, unsigned depth, unsigned mipmapCount, bool autoMipMapCount)
{
	auto impl = (Texture3DGL*)mImpl.get();
	impl->resize(width, height, depth, mipmapCount, autoMipMapCount);
}

nex::Texture3DGL::Texture3DGL(GLuint width, GLuint height, GLuint depth, const TextureDesc& textureData, const TextureTransferDesc* desc) :
Impl(GL_FALSE, TextureTarget::TEXTURE3D, textureData, width, height, depth, 1)
{
	Impl::generateTexture(&mTextureID, textureData, (GLenum)mTargetGL);

	resize(mWidth, mHeight, mDepth, 1, mTextureData.generateMipMaps);

	// set base mip map
	if (desc) upload(*desc);


	// create content of the other mipmaps
	if (mTextureData.generateMipMaps) generateMipMaps();
	else updateMipMapCount();
}

nex::Texture3DGL::Texture3DGL(GLuint texture, const TextureDesc& textureData, unsigned width, unsigned height, unsigned depth)
	: Impl(texture, TextureTarget::TEXTURE3D, textureData, width, height, depth, 1)
{
	updateMipMapCount();
}

void nex::Texture3DGL::resize(unsigned width, unsigned height, unsigned depth, unsigned mipmapCount, bool autoMipMapCount)
{
	mWidth = width;
	mHeight = height;
	mDepth = depth;
	mTextureData.generateMipMaps = autoMipMapCount;

	resizeTexImage3D(mTextureID,
		mipmapCount,
		mWidth,
		mHeight,
		mDepth,
		(GLenum)translate(mTextureData.internalFormat),
		autoMipMapCount);

	updateMipMapCount();
}

void nex::Texture3DGL::upload(const TextureTransferDesc& desc)
{
	// On some cards glTextureSubImage2D crashes when data is nullptr
	if (!desc.data) return;

	const auto& imageDesc = desc.imageDesc;

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, imageDesc.rowByteAlignmnet));

	GLCall(glTextureSubImage3D(mTextureID, desc.mipMapLevel,
		desc.xOffset, desc.yOffset, desc.zOffset,
		imageDesc.width, imageDesc.height, imageDesc.depth,
		(GLenum)translate(imageDesc.colorspace),
		(GLenum)translate(imageDesc.pixelDataType),
		desc.data));

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, PIXEL_STORE_DEFAULT_ALIGNMENT));
}

nex::CubeMapArray::CubeMapArray(std::unique_ptr<Impl> impl) : Texture(std::move(impl))
{
}

nex::CubeMapArray::CubeMapArray(unsigned sideWidth, unsigned sideHeight, unsigned depth, const TextureDesc & textureData, const TextureTransferDesc* desc)
	: Texture(make_unique<CubeMapArrayGL>(sideWidth, sideHeight, depth, textureData, desc))
{
}

unsigned nex::CubeMapArray::getLayerFaces()
{
	return ((CubeMapArrayGL*)mImpl.get())->getLayerFaces();
}

unsigned nex::CubeMapArray::getLayerFaceIndex(unsigned arrayIndex)
{
	return arrayIndex * 6;
}

void nex::CubeMapArray::resize(unsigned sideWidth, unsigned sideHeight, unsigned depth, unsigned mipmapCount, bool autoMipMapCount)
{
	((CubeMapArrayGL*)mImpl.get())->resize(sideWidth, sideHeight, depth, mipmapCount, autoMipMapCount);
}

nex::CubeMapArrayGL::CubeMapArrayGL(GLuint sideWidth, GLuint sideHeight, GLuint depth, const TextureDesc & textureData, const TextureTransferDesc* desc)
 : Impl(GL_FALSE, TextureTarget::CUBE_MAP_ARRAY, textureData, sideWidth, sideHeight, depth, 1), mLayerFaces(depth * 6)
{
	Impl::generateTexture(&mTextureID, textureData, (GLenum)mTargetGL);

	resize(mWidth,mHeight, mDepth, 1, mTextureData.generateMipMaps);

	if (desc) upload(*desc);

	// create content of the other mipmaps
	if (mTextureData.generateMipMaps) generateMipMaps();
	else updateMipMapCount();
}

nex::CubeMapArrayGL::CubeMapArrayGL(GLuint texture, const TextureDesc & textureData, unsigned sideWidth, unsigned sideHeight, unsigned depth)
	: Impl(texture, TextureTarget::CUBE_MAP_ARRAY, textureData, sideWidth, sideHeight, depth, 1), mLayerFaces(depth * 6)
{
	updateMipMapCount();
}

unsigned nex::CubeMapArrayGL::getLayerFaces() const
{
	return mLayerFaces;
}

void nex::CubeMapArrayGL::resize(unsigned sideWidth, unsigned sideHeight, unsigned depth, unsigned mipmapCount, bool autoMipMapCount)
{
	mWidth = sideWidth;
	mHeight = sideHeight;
	mDepth = depth;
	mLayerFaces = mDepth * 6;

	mTextureData.generateMipMaps = autoMipMapCount;

	resizeTexImage3D(mTextureID,
		mipmapCount,
		mWidth,
		mHeight,
		mLayerFaces,
		(GLenum)translate(mTextureData.internalFormat),
		autoMipMapCount);

	updateMipMapCount();
}

void nex::CubeMapArrayGL::upload(const TextureTransferDesc& desc)
{
	GLCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
	// On some cards glTextureSubImage2D crashes when data is nullptr
	if (!desc.data) return;

	const auto& imageDesc = desc.imageDesc;

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, imageDesc.rowByteAlignmnet));

	GLCall(glTextureSubImage3D(mTextureID, desc.mipMapLevel,
		desc.xOffset, desc.yOffset, desc.zOffset,
		imageDesc.width, imageDesc.height, imageDesc.depth,
		(GLenum)translate(imageDesc.colorspace),
		(GLenum)translate(imageDesc.pixelDataType),
		desc.data));

	GLCall(glPixelStorei(GL_PACK_ALIGNMENT, PIXEL_STORE_DEFAULT_ALIGNMENT));
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
		ChannelGL::ONE,
		ChannelGL::ZERO,
	};

	static const unsigned size = (unsigned)Channel::LAST - (unsigned)Channel::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: Channel and ChannelGL don't match!");

	return table[(unsigned)channel];
}

nex::TextureFilterGL nex::translate(nex::TexFilter filter)
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

	static const unsigned size = (unsigned)TexFilter::LAST - (unsigned)TexFilter::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureFilter and TextureFilterGL don't match!");

	return table[(unsigned)filter];
}

nex::TextureUVTechniqueGL nex::translate(nex::UVTechnique technique)
{
	static TextureUVTechniqueGL const table[]
	{
		TextureUVTechniqueGL::ClampToBorder,
		TextureUVTechniqueGL::ClampToEdge,
		TextureUVTechniqueGL::MirrorRepeat,
		TextureUVTechniqueGL::MirrorClampToEdge,
		TextureUVTechniqueGL::Repeat,
	};

	static const unsigned size = (unsigned)UVTechnique::LAST - (unsigned)UVTechnique::FIRST + 1;
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
		ColorSpaceGL::BGR,
		ColorSpaceGL::RGB_INTEGER,
		ColorSpaceGL::BGR_INTEGER,

		ColorSpaceGL::RGBA,
		ColorSpaceGL::BGRA,
		ColorSpaceGL::RGBA_INTEGER,
		ColorSpaceGL::BGRA_INTEGER,

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

nex::DepthStencilTextureModeGL nex::translate(nex::DepthStencilTexMode mode)
{
	static DepthStencilTextureModeGL const table[]{
		DepthStencilTextureModeGL::DEPTH,
		DepthStencilTextureModeGL::STENCIL,
	};
	static const unsigned size = (unsigned)DepthStencilTexMode::LAST - (unsigned)DepthStencilTexMode::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: DepthStencilTextureMode and DepthStencilTextureModeGL don't match!");

	return table[(unsigned)mode];
}

nex::InternFormatGL nex::translate(nex::InternalFormat format)
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

		InternFormatGL::RGB5,
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

		InternFormatGL::RGB10_A2,
		InternFormatGL::RGB10_A2UI,

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

	static const unsigned size = (unsigned)InternalFormat::LAST - (unsigned)InternalFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: InternFormat and InternFormatGL don't match!");

	return table[(unsigned)format];
}

nex::PixelDataTypeGL nex::translate(nex::PixelDataType dataType)
{
	static PixelDataTypeGL const table[]
	{
		PixelDataTypeGL::FLOAT,
		PixelDataTypeGL::FLOAT_HALF,
		PixelDataTypeGL::INT,
		PixelDataTypeGL::UBYTE,
		PixelDataTypeGL::UINT,
		PixelDataTypeGL::SHORT,

		PixelDataTypeGL::UNSIGNED_INT_24_8,
		PixelDataTypeGL::FLOAT_32_UNSIGNED_INT_24_8_REV,
		PixelDataTypeGL::UNSIGNED_SHORT,
		PixelDataTypeGL::UNSIGNED_INT_24,
		PixelDataTypeGL::UNSIGNED_INT_8_8_8_8,

		PixelDataTypeGL::UNSIGNED_INT_10_10_10_2,
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
		TextureTargetGl::CUBE_MAP_ARRAY,

		TextureTargetGl::RENDERBUFFER,
	};

	static const unsigned size = (unsigned)TextureTarget::LAST - (unsigned)TextureTarget::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: TextureTarget and TextureTargetGl don't match!");

	return table[(unsigned)target];
}