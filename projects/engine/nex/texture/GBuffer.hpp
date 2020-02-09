#pragma once

#include <nex/texture/RenderTarget.hpp>

namespace nex
{
	class PBR_GBuffer : public RenderTarget
	{
	public:

		// Has to be implemented by renderer backend
		PBR_GBuffer(unsigned width, unsigned height);

		Texture2D* getAlbedo() const;
		Texture2D* getAoMetalRoughness() const;
		Texture2D* getMotion() const;
		Texture2D* getNormal() const;
		Texture2D* getDepth() const;

		Texture2D* getEmissionPerObjectMaterialID() const;

		const RenderAttachment& getDepthRenderTarget() const;
		const RenderAttachment& getMotionRenderTarget() const;

	private:
		Texture2D* mAlbedo;
		Texture2D* mAoMetalRoughness;
		Texture2D* mMotion;
		Texture2D* mNormal;
		Texture2D* mDepth;
		Texture2D* mEmissionPerObjectMaterialID;

	};
}