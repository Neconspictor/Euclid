#pragma once

#include <nex/texture/RenderTarget.hpp>

namespace nex
{
	class PBR_GBuffer : public RenderTarget
	{
	public:

		// Has to be implemented by renderer backend
		PBR_GBuffer(unsigned width, unsigned height);

		Texture* getAlbedo() const;
		Texture* getAoMetalRoughness() const;
		Texture* getNormal() const;
		Texture* getDepth() const;

	private:
		Texture* mAlbedo;
		Texture* mAoMetalRoughness;
		Texture* mNormal;
		Texture* mDepth;
	};
}