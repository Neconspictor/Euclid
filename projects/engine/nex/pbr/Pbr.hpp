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
	class PbrGeometryPass;
	class GlobalIllumination;

	class Pbr {

	public:
		Pbr(AmbientLight* ambientLight, 
			GlobalIllumination* globalIllumination,
			CascadedShadow* cascadeShadow, DirectionalLight* dirLight);

		virtual ~Pbr();

		AmbientLight* getAmbientLight();

		CascadedShadow* getCascadedShadow();

		DirectionalLight* getDirLight();

		GlobalIllumination* getGlobalIllumination();

		virtual void reloadLightingShader(CascadedShadow* cascadedShadow) = 0;

		void setAmbientLight(AmbientLight * light);

		void setCascadedShadow(CascadedShadow* shadow);

		void setDirLight(DirectionalLight * light);

	protected:
		AmbientLight* mAmbientLight;
		CascadedShadow* mCascadeShadow;
		DirectionalLight* mLight;
		GlobalIllumination* mGlobalIllumination;
	};

	class PbrTechnique : public Technique
	{
	public:
		PbrTechnique(AmbientLight* ambientLight, GlobalIllumination* globalIllumination, CascadedShadow* cascadeShadow, DirectionalLight* dirLight);
		virtual ~PbrTechnique();


		void useDeferred();
		void useForward();
		PbrDeferred* getDeferred();
		PbrForward* getForward();
		Pbr* getActive();

		PbrGeometryPass* getActiveGeometryPass();
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
