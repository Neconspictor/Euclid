#include <nex/gui/vob/RiggedVobView.hpp>
#include <nex/scene/Vob.hpp>

bool nex::gui::RiggedVobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
{
	if (!VobView::draw(vob, scene, picker, camera, doOneTimeChanges)) {
		return false;
	}

	auto* riggedVob = dynamic_cast<RiggedVob*>(vob);

	if (!riggedVob) {
		return true;
	}

	return true;
}
