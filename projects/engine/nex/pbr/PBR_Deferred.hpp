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
			Camera* camera,
			Texture* ssaoMap);

		void drawSky(Camera* camera);

		std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height);

		float getAmbientLightPower() const;

		DirectionalLight* getDirLight();

		void reloadLightingShader(const CascadedShadow& cascadedShadow);

		void setAmbientLightPower(float power);

		void setDirLight(DirectionalLight * light);

	private:
		Sprite screenSprite;
		std::unique_ptr<PBRShader_Deferred_Geometry> mGeometryPass;
		std::unique_ptr<PBRShader_Deferred_Lighting> mLightPass;
		CascadedShadow* mCascadeShadow;
		float mAmbientLightPower;
		DirectionalLight* mLight;
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