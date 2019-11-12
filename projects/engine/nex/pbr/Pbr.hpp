#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/shader/Technique.hpp>
#include "nex/camera/Camera.hpp"
#include "nex/texture/GBuffer.hpp"
#include <memory>

namespace nex
{
	class CascadedShadow;
	struct DirLight;
	class PbrProbe;
	class PbrDeferred;
	class PbrForward;
	class PbrGeometryPass;
	class GlobalIllumination;

	class Pbr {

	public:
		Pbr(GlobalIllumination* globalIllumination,
			CascadedShadow* cascadeShadow, DirLight* dirLight);

		virtual ~Pbr();

		CascadedShadow* getCascadedShadow();

		DirLight* getDirLight();

		GlobalIllumination* getGlobalIllumination();

		virtual void reloadLightingShaders() = 0;

		void setCascadedShadow(CascadedShadow* shadow);

		void setGI(GlobalIllumination*);

		void setDirLight(DirLight* light);

	protected:
		CascadedShadow* mCascadedShadow;
		DirLight* mLight;
		GlobalIllumination* mGlobalIllumination;
	};

	class PbrTechnique : public Technique
	{
	public:
		PbrTechnique(GlobalIllumination* globalIllumination, CascadedShadow* cascadeShadow, DirLight* dirLight);
		virtual ~PbrTechnique();


		void useDeferred();
		void useForward();
		PbrDeferred* getDeferred();
		PbrForward* getForward();
		Pbr* getActive();

		PbrGeometryPass* getActiveGeometryPass();

		void overrideForward(PbrForward* forward);
		void overrideDeferred(PbrDeferred* deferred);

		void setGI(GlobalIllumination* globalIllumination);
		void setShadow(CascadedShadow* cascadeShadow);
		void setDirLight(DirLight* dirLight);

		void updateShaders();

	private:
		std::unique_ptr<PbrDeferred> mDeferred;
		std::unique_ptr<PbrForward> mForward;
		PbrDeferred* mOverrideDeferred;
		PbrForward* mOverrideForward;
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
