#pragma once

#include <nex/texture/Texture.hpp>
#include <nex/gui/Drawable.hpp>
#include <nex/pbr/PBR.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/shader/PBRShader.hpp>

namespace nex
{
	class SceneNode;

	class PBR_GBuffer;

	class PBR_Deferred : public PBR {

	public:
		PBR_Deferred(Texture* backgroundHDR, CascadedShadow* cascadeShadow);

		void drawGeometryScene(SceneNode * scene,
			const glm::mat4& view,
			const glm::mat4& projection);

		void drawLighting(SceneNode * scene,
			PBR_GBuffer* gBuffer,
			Texture* ssaoMap,
			const DirectionalLight& light,
			const glm::mat4& viewFromGPass,
			const glm::mat4& projFromGPass,
			const glm::mat4& worldToLight,
			CascadedShadow::CascadeData* cascadeData,
			Texture* cascadedDepthMap);

		void drawSky(const glm::mat4& projection,
			const glm::mat4& view);

		std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height);

		void reloadLightingShader(unsigned csmNumCascades,
			const CascadedShadow::PCFFilter& pcf);

	private:
		Sprite screenSprite;
		std::unique_ptr<PBRShader_Deferred_Geometry> mGeometryPass;
		std::unique_ptr<PBRShader_Deferred_Lighting> mLightPass;
		CascadedShadow* mCascadeShadow;
	};

	class PBR_Deferred_ConfigurationView : public nex::gui::Drawable {
	public:
		PBR_Deferred_ConfigurationView(PBR_Deferred* pbr);

	protected:
		void drawSelf() override;

	private:
		PBR_Deferred * m_pbr;
	};
}