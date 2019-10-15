#include <nex/texture/GBuffer.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include "Texture.hpp"
#include "Attachment.hpp"

using namespace std;

namespace nex
{
	PBR_GBuffer::PBR_GBuffer(unsigned width, unsigned height) : RenderTarget(width, height),
	mAlbedo(nullptr), mAoMetalRoughness(nullptr), mNormal(nullptr), mNormalizedViewSpaceZ(nullptr)
	{
		bind();

		TextureDesc data;
		data.minFilter = TexFilter::Nearest;
		data.magFilter = TexFilter::Nearest;
		data.wrapR = UVTechnique::ClampToEdge;
		data.wrapS = UVTechnique::ClampToEdge;
		data.wrapT = UVTechnique::ClampToEdge;
		data.maxAnisotropy = 1.0f;
		data.generateMipMaps = false;
		data.useSwizzle = false;

		RenderAttachment temp;

		// albedo
		data.colorspace = ColorSpace::RGB;
		data.pixelDataType = PixelDataType::UBYTE;
		data.internalFormat = InternalFormat::RGB8;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.colorAttachIndex = 0;
		mAlbedo = static_cast<Texture2D*>(temp.texture.get());
		addColorAttachment(temp);

		// ao metal roughness
		data.internalFormat = InternalFormat::RGB5;
		data.pixelDataType = PixelDataType::FLOAT;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.colorAttachIndex = 1;
		mAoMetalRoughness = static_cast<Texture2D*>(temp.texture.get());
		addColorAttachment(temp);


		// normal
		data.colorspace = ColorSpace::RGBA;
		data.internalFormat = InternalFormat::RGB10_A2;
		data.pixelDataType = PixelDataType::UNSIGNED_INT_10_10_10_2;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.colorAttachIndex = 2;
		mNormal = static_cast<Texture2D*>(temp.texture.get());
		addColorAttachment(temp);

		// normalized viewspace z
		data.internalFormat = InternalFormat::R32F;
		data.colorspace = ColorSpace::R;
		//data.colorspace = ColorSpace::DEPTH;
		data.pixelDataType = PixelDataType::FLOAT;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.colorAttachIndex = 3;
		mNormalizedViewSpaceZ = static_cast<Texture2D*>(temp.texture.get());
		addColorAttachment(temp);

		// motion
		data.internalFormat = InternalFormat::RG16F;
		data.colorspace = ColorSpace::RG;
		data.pixelDataType = PixelDataType::FLOAT;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);
		temp.colorAttachIndex = 4;
		mMotion = static_cast<Texture2D*>(temp.texture.get());
		addColorAttachment(temp);


		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		finalizeAttachments();

		// create and attach depth buffer (renderbuffer)
		// depth/stencil
		data.minFilter = TexFilter::Nearest;
		data.magFilter = TexFilter::Nearest;
		data.wrapS = UVTechnique::ClampToEdge;
		data.wrapT = UVTechnique::ClampToEdge;
		data.internalFormat = InternalFormat::DEPTH24_STENCIL8;
		data.pixelDataType = PixelDataType::UNSIGNED_INT_24_8;
		data.colorspace = ColorSpace::DEPTH_STENCIL;

		temp.type = RenderAttachmentType::DEPTH_STENCIL;
		temp.texture = make_shared<Texture2D>(width, height, data, nullptr);

		useDepthAttachment(std::move(temp));

		// finally check if framebuffer is complete
		assertCompletion();
	}

	Texture2D* PBR_GBuffer::getAlbedo() const
	{
		return mAlbedo;
	}

	Texture2D* PBR_GBuffer::getAoMetalRoughness() const
	{
		return mAoMetalRoughness;
	}

	Texture2D* PBR_GBuffer::getMotion() const
	{
		return mMotion;
	}

	Texture2D* PBR_GBuffer::getNormal() const
	{
		return mNormal;
	}

	Texture2D* PBR_GBuffer::getDepth() const
	{
		return mNormalizedViewSpaceZ;
	}

	const RenderAttachment& PBR_GBuffer::getDepthRenderTarget() const
	{
		return getColorAttachments()[3];
	}

	const RenderAttachment& PBR_GBuffer::getMotionRenderTarget() const
	{
		return getColorAttachments()[4];
	}
}