#pragma once

#include <nex/gui/vob/VobView.hpp>
#include <nex/gui/TextureView.hpp>

namespace nex::gui
{

	class Picker;

	class OceanVobView : public VobView {
	public:

		OceanVobView(nex::Window* window);
		virtual ~OceanVobView() = default;

		bool draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges) override;

	private:
	};
}