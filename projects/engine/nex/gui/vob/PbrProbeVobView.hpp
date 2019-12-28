#pragma once

#include <nex/gui/vob/VobView.hpp>
#include <nex/gui/TextureView.hpp>

namespace nex::gui
{

	class Picker;

	class PbrProbeVobView : public VobView {
	public:

		PbrProbeVobView();
		virtual ~PbrProbeVobView() = default;

		void draw(Vob* vob, Scene* scene, Picker* picker, bool doOneTimeChanges) override;

	private:
		TextureView mBrdfView;
		TextureView mConvolutedView;
		TextureView mPrefilteredView;
	};
}