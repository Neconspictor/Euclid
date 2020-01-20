#include <nex/gui/vob/VobViewMapper.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/GI/Probe.hpp>
#include <nex/water/Ocean.hpp>
#include <nex/gui/vob/OceanVobView.hpp>
#include <nex/gui/vob/ProbeVobView.hpp>
#include <nex/gui/vob/RiggedVobView.hpp>


std::unique_ptr<nex::gui::ProbeVobView> nex::gui::VobViewMapper::mProbeVobView = nullptr;

void nex::gui::VobViewMapper::init(ProbeManager* probeManager)
{
	mProbeVobView = std::make_unique<ProbeVobView>(probeManager);
}

nex::gui::VobView* nex::gui::VobViewMapper::getViewByVob(Vob* vob)
{
	static VobView vobView;
	static OceanVobView oceanVobView;
	static RiggedVobView riggedVobView;

	if (dynamic_cast<nex::ProbeVob*>(vob)) {
		return mProbeVobView.get();
	}
	else if (dynamic_cast<nex::OceanVob*>(vob)) {
		return &oceanVobView;
	}
	else if (dynamic_cast<nex::RiggedVob*>(vob)) {
		return &riggedVobView;
	}

	return &vobView;
}