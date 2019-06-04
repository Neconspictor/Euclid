#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/shader/Technique.hpp>
#include "nex/camera/Camera.hpp"
#include "nex/texture/GBuffer.hpp"
#include <memory>

namespace nex
{
	class CascadedShadow;
	class DirectionalLight;
	class AmbientLight;
	class PbrProbe;
	class PbrDeferred;
	class PbrForward;

	class Pbr {

	public:
		Pbr(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe);

		virtual ~Pbr();

		AmbientLight* getAmbientLight();

		CascadedShadow* getCascadedShadow();

		DirectionalLight* getDirLight();

		PbrProbe* getProbe();

		virtual void reloadLightingShader(CascadedShadow* cascadedShadow) = 0;

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

	class PbrTechnique : public Technique
	{
	public:
		PbrTechnique(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe);
		virtual ~PbrTechnique();


		void useDeferred();
		void useForward();
		PbrDeferred* getDeferred();
		PbrForward* getForward();
		Pbr* getActive();
		void setProbe(PbrProbe* probe);
	private:
		std::unique_ptr<PbrDeferred> mDeferred;
		std::unique_ptr<PbrForward> mForward;
		bool mDeferredUsed;
	};

	class Pbr_ConfigurationView : public nex::gui::Drawable {
	public:
		Pbr_ConfigurationView(PbrTechnique* pbr);

	protected:
		void drawSelf() override;

		void drawLightSphericalDirection();

	private:
		PbrTechnique* mPbr;
	};
}
