#pragma once

#include <nex/gui/vob/VobView.hpp>

namespace nex::gui
{

	class RiggedVobView : public VobView {
	public:

		RiggedVobView() = default;
		virtual ~RiggedVobView() = default;

		bool draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges) override;

	protected:
	};
}