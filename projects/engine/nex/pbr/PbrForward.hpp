#pragma once

#include <nex/gui/Drawable.hpp>

namespace nex
{
	class Texture;
	class Camera;
	class CascadedShadow;
	class SceneNode;
	class DirectionalLight;
	class PBRShader;

	class PbrProbe;
	class Sampler;

	class PbrForward  {

	public:
		PbrForward(PbrProbe* probe, DirectionalLight* dirLight, CascadedShadow* cascadeShadow);

		void drawLighting(SceneNode * scene,
			Camera* camera);

		void drawSky(Camera* camera);

		float getAmbientLightPower() const;

		DirectionalLight* getDirLight();

		void reloadLightingShader(const CascadedShadow& cascadedShadow);

		void setAmbientLightPower(float power);

		void setDirLight(DirectionalLight * light);

	private:
		std::unique_ptr<PBRShader> mForwardShader;
		std::unique_ptr<Sampler> mPointSampler;
		CascadedShadow* mCascadeShadow;
		float mAmbientLightPower;
		DirectionalLight* mLight;
		PbrProbe* mProbe;
	};

	class PbrForward_ConfigurationView : public nex::gui::Drawable {
	public:
		PbrForward_ConfigurationView(PbrForward* pbr);

	protected:
		void drawSelf() override;

		void drawLightSphericalDirection();

	private:
		PbrForward * mPbr;
	};
}