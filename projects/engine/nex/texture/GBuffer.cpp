#include <nex/texture/GBuffer.hpp>
#include <nex/util/ExceptionHandling.hpp>

using namespace std;

namespace nex
{
	PBR_GBuffer::PBR_GBuffer(unsigned width, unsigned height) : RenderTarget(),
	mAlbedo(nullptr), mAoMetalRoughness(nullptr), mNormal(nullptr), mDepth(nullptr)
	{
		bind();

		TextureData data;
		data.minFilter = TextureFilter::NearestNeighbor;
		data.magFilter = TextureFilter::NearestNeighbor;
		data.wrapR = TextureUVTechnique::ClampToEdge;
		data.wrapS = TextureUVTechnique::ClampToEdge;
		data.wrapT = TextureUVTechnique::ClampToEdge;
		data.generateMipMaps = false;
		data.useSwizzle = false;

		RenderAttachment temp;

		// albedo
		data.colorspace = ColorSpace::RGB;
		data.pixelDataType = PixelDataType::FLOAT;
		data.internalFormat = InternFormat::RGB16F;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.colorAttachIndex = 0;
		mAlbedo = temp.texture.get();
		addColorAttachment(temp);

		// ao metal roughness
		data.internalFormat = InternFormat::RGB8;
		data.pixelDataType = PixelDataType::UBYTE;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.colorAttachIndex = 1;
		mAoMetalRoughness = temp.texture.get();
		addColorAttachment(temp);


		// normal
		data.internalFormat = InternFormat::RGB16F;
		data.pixelDataType = PixelDataType::FLOAT;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.colorAttachIndex = 2;
		mNormal = temp.texture.get();
		addColorAttachment(temp);

		// depth
		data.internalFormat = InternFormat::R32F;
		data.pixelDataType = PixelDataType::FLOAT;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.colorAttachIndex = 3;
		mDepth = temp.texture.get();
		addColorAttachment(temp);


		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		finalizeAttachments();

		// create and attach depth buffer (renderbuffer)
		// depth/stencil
		data.minFilter = TextureFilter::NearestNeighbor;
		data.magFilter = TextureFilter::NearestNeighbor;
		data.wrapS = TextureUVTechnique::ClampToEdge;
		data.wrapT = TextureUVTechnique::ClampToEdge;
		data.internalFormat = InternFormat::DEPTH24_STENCIL8;
		data.pixelDataType = PixelDataType::UNSIGNED_INT_24_8;
		data.colorspace = ColorSpace::DEPTH_STENCIL;

		temp.type = RenderAttachment::Type::DEPTH_STENCIL;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);

		useDepthAttachment(std::move(temp));

		// finally check if framebuffer is complete
		assertCompletion();
	}

	Texture* PBR_GBuffer::getAlbedo() const
	{
		return mAlbedo;
	}

	Texture* PBR_GBuffer::getAoMetalRoughness() const
	{
		return mAoMetalRoughness;
	}

	Texture* PBR_GBuffer::getNormal() const
	{
		return mNormal;
	}

	Texture* PBR_GBuffer::getDepth() const
	{
		return mDepth;
	}
}