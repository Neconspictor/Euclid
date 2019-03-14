#pragma once

#include <nex/texture/TextureSamplerData.hpp>
#include <memory>

namespace nex
{
	class Texture;

	enum class RenderAttachmentType
	{
		COLOR, FIRST = COLOR,
		DEPTH,
		STENCIL,
		DEPTH_STENCIL, LAST = DEPTH_STENCIL,
	};

	struct RenderAttachment
	{

		unsigned colorAttachIndex = 0; // only used for color attachments
		unsigned mipmapLevel = 0;
		unsigned layer = 0; // Specifies the layer of an array texture; Must be zero for non array textures;
		CubeMapSide side = CubeMapSide::POSITIVE_X;  // only used when target is TextureTarget::CubeMap
		TextureTarget target = TextureTarget::TEXTURE2D;
		RenderAttachmentType type = RenderAttachmentType::COLOR;
		std::shared_ptr<Texture> texture;

		RenderAttachment() {}

		RenderAttachment(unsigned attachIndex, unsigned mipmapLevel, unsigned layer, CubeMapSide side, TextureTarget target, RenderAttachmentType type, std::shared_ptr<Texture> texture)
			:
			colorAttachIndex(attachIndex),
			mipmapLevel(mipmapLevel),
			layer(layer),
			side(side),
			target(target),
			type(type),
			texture(std::move(texture))
		{}

		static RenderAttachmentType translate(InternFormat format);

	};
}
