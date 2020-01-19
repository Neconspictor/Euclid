#pragma once

#include <nex/gui/vob/VobView.hpp>
#include <nex/gui/TextureView.hpp>

namespace nex {
	class ProbeManager;
}

namespace nex::gui
{

	class Picker;

	class PbrProbeVobView : public VobView {
	public:

		PbrProbeVobView(ProbeManager* probeManager);
		virtual ~PbrProbeVobView() = default;

		bool draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges) override;

	private:
		TextureView mBrdfView;
		TextureView mIrradianceView;
		TextureView mReflectionView;
		ProbeManager* mProbeManager;
	};
}