#pragma once

#include <nex/gui/vob/VobView.hpp>
#include <nex/gui/TextureView.hpp>

namespace nex::gui
{

	class Picker;

	class OceanVobView : public VobView {
	public:

		OceanVobView();
		virtual ~OceanVobView() = default;

		void draw(Vob* vob, Scene* scene, Picker* picker, bool doOneTimeChanges) override;

	private:
	};
}