#include <nex/texture/GBuffer.hpp>
#include <nex/util/ExceptionHandling.hpp>

using namespace std;

namespace nex
{
	PBR_GBuffer::PBR_GBuffer(unsigned width, unsigned height) : RenderTarget(width, height),
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
		temp.attachIndex = 0;
		mAlbedo = temp.texture.get();
		addAttachment(temp);

		// ao metal roughness
		data.internalFormat = InternFormat::RGB8;
		data.pixelDataType = PixelDataType::UBYTE;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.attachIndex = 1;
		mAoMetalRoughness = temp.texture.get();
		addAttachment(temp);


		// normal
		data.internalFormat = InternFormat::RGB16F;
		data.pixelDataType = PixelDataType::FLOAT;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.attachIndex = 2;
		mNormal = temp.texture.get();
		addAttachment(temp);

		// depth
		data.internalFormat = InternFormat::R32F;
		data.pixelDataType = PixelDataType::FLOAT;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.attachIndex = 3;
		mDepth = temp.texture.get();
		addAttachment(temp);


		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		updateAttachments();

		// create and attach depth buffer (renderbuffer)
		// depth/stencil
		DepthStencilDesc desc;
		desc.minFilter = TextureFilter::NearestNeighbor;
		desc.magFilter = TextureFilter::NearestNeighbor;
		desc.wrap = TextureUVTechnique::ClampToEdge;
		desc.format = DepthStencilFormat::DEPTH24_STENCIL8;
		auto depthBuffer = make_shared<DepthStencilMap>(width, height, desc);

		useDepthStencilMap(std::move(depthBuffer));

		// finally check if framebuffer is complete
		if (!isComplete())
			throw_with_trace(std::runtime_error("PBR_GBuffer::PBR_GBuffer: Couldn't successfully init framebuffer!"));

		unbind();
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