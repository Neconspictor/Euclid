#pragma once

#include <nex/gui/Drawable.hpp>

namespace nex
{
	class Texture;
	class Camera;
	class CascadedShadow;
	class DirectionalLight;
	class SceneNode;

	class PBR_GBuffer;
	class Sampler;
	class PbrProbe;
	class PBRShader_Deferred_Geometry;
	class PBRShader_Deferred_Lighting;

	class PbrDeferred {

	public:
		PbrDeferred(PbrProbe* probe, DirectionalLight* dirLight, CascadedShadow* cascadeShadow);

		void drawGeometryScene(SceneNode * scene, Camera* camera);

		void drawLighting(SceneNode * scene,
			PBR_GBuffer* gBuffer,
			Camera* camera);

		void drawSky(Camera* camera);

		std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height);

		float getAmbientLightPower() const;

		DirectionalLight* getDirLight();

		void reloadLightingShader(const CascadedShadow& cascadedShadow);

		void setAmbientLightPower(float power);

		void setDirLight(DirectionalLight * light);

	private:
		std::unique_ptr<PBRShader_Deferred_Geometry> mGeometryPass;
		std::unique_ptr<PBRShader_Deferred_Lighting> mLightPass;
		std::unique_ptr<Sampler> mPointSampler;
		CascadedShadow* mCascadeShadow;
		float mAmbientLightPower;
		DirectionalLight* mLight;
		PbrProbe* mProbe;
	};

	class PBR_Deferred_ConfigurationView : public nex::gui::Drawable {
	public:
		PBR_Deferred_ConfigurationView(PbrDeferred* pbr);

	protected:
		void drawSelf() override;

		void drawLightSphericalDirection();

	private:
		PbrDeferred * mPbr;
	};
}