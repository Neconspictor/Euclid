#include <nex/gui/vob/VobViewMapper.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/GI/PbrProbe.hpp>
#include <nex/water/Ocean.hpp>
#include <nex/gui/vob/OceanVobView.hpp>
#include <nex/gui/vob/PbrProbeVobView.hpp>
#include <nex/gui/vob/RiggedVobView.hpp>

nex::gui::VobView* nex::gui::VobViewMapper::getViewByVob(Vob* vob)
{
	static VobView vobView;
	static OceanVobView oceanVobView;
	static PbrProbeVobView pbrProbeView;
	static RiggedVobView riggedVobView;

	if (dynamic_cast<nex::ProbeVob*>(vob)) {
		return &pbrProbeView;
	}
	else if (dynamic_cast<nex::OceanVob*>(vob)) {
		return &oceanVobView;
	}
	else if (dynamic_cast<nex::RiggedVob*>(vob)) {
		return &riggedVobView;
	}

	return &vobView;
}