#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/shader/Technique.hpp>

namespace nex
{
	class CascadedShadow;
	class DirectionalLight;
	class AmbientLight;
	class PbrProbe;

	class Pbr : public Technique {

	public:
		Pbr(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe, Pass* submeshPass);

		virtual ~Pbr();

		AmbientLight* getAmbientLight();

		CascadedShadow* getCascadedShadow();

		DirectionalLight* getDirLight();

		PbrProbe* getProbe();

		virtual void reloadLightingShader(const CascadedShadow& cascadedShadow) = 0;

		void setAmbientLight(AmbientLight * light);

		void setCascadedShadow(CascadedShadow* shadow);
		
		void setDirLight(DirectionalLight * light);

		void setProbe(PbrProbe* probe);

	protected:
		AmbientLight* mAmbientLight;
		CascadedShadow* mCascadeShadow;
		DirectionalLight* mLight;
		PbrProbe* mProbe;
	};

	class Pbr_ConfigurationView : public nex::gui::Drawable {
	public:
		Pbr_ConfigurationView(Pbr* pbr);

	protected:
		void drawSelf() override;

		void drawLightSphericalDirection();

	private:
		Pbr* mPbr;
	};
}
