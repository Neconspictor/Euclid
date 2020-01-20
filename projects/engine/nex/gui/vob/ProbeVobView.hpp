#pragma once

#include <nex/gui/vob/VobView.hpp>
#include <nex/gui/TextureView.hpp>

namespace nex {
	class ProbeManager;
}

namespace nex::gui
{

	class Picker;

	class ProbeVobView : public VobView {
	public:

		ProbeVobView(ProbeManager* probeManager);
		virtual ~ProbeVobView() = default;

		bool draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges) override;

	private:
		TextureView mBrdfView;
		TextureView mIrradianceView;
		TextureView mReflectionView;
		ProbeManager* mProbeManager;
	};
}