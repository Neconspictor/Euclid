#pragma once

#include <nex/gui/Drawable.hpp>
#include "nex/camera/Camera.hpp"
#include "nex/texture/GBuffer.hpp"
#include <memory>
#include <nex/shadow/CascadedShadow.hpp>

namespace nex
{
	class CascadedShadow;
	struct DirLight;
	class PbrProbe;
	class PbrDeferred;
	class PbrForward;
	class PbrGeometryShader;
	class GlobalIllumination;

	class Pbr {

	public:
		Pbr(GlobalIllumination* globalIllumination,
			CascadedShadow* cascadeShadow, const DirLight* dirLight);

		virtual ~Pbr();

		CascadedShadow* getCascadedShadow();

		GlobalIllumination* getGlobalIllumination();

		virtual void reloadLightingShaders() = 0;

		void setCascadedShadow(CascadedShadow* shadow);

		void setGI(GlobalIllumination*);

		void setDirLight(const DirLight* light);

	protected:
		CascadedShadow* mCascadedShadow;
		const DirLight* mLight;
		GlobalIllumination* mGlobalIllumination;
		CascadedShadow::ChangedCallback::Handle mCascadeChangedHandle;
	};

	class PbrTechnique
	{
	public:
		PbrTechnique(GlobalIllumination* globalIllumination, CascadedShadow* cascadeShadow, DirLight* dirLight);
		virtual ~PbrTechnique();

		PbrDeferred* getDeferred();
		PbrForward* getForward();
		Pbr* getActive();

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
		Pbr_ConfigurationView(PbrTechnique* pbr, DirLight* light);

	protected:
		void drawSelf() override;

		void drawLightSphericalDirection();

	private:
		PbrTechnique* mPbr;
		DirLight* mLight;
	};
}