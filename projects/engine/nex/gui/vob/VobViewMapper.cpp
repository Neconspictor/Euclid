#include <nex/gui/vob/VobViewMapper.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/GI/PbrProbe.hpp>
#include <nex/water/Ocean.hpp>
#include <nex/gui/vob/OceanVobView.hpp>
#include <nex/gui/vob/PbrProbeVobView.hpp>
#include <nex/gui/vob/RiggedVobView.hpp>


std::unique_ptr<nex::gui::PbrProbeVobView> nex::gui::VobViewMapper::mPbrProbeVobView = nullptr;

void nex::gui::VobViewMapper::init(ProbeManager* probeManager)
{
	mPbrProbeVobView = std::make_unique<PbrProbeVobView>(probeManager);
}

nex::gui::VobView* nex::gui::VobViewMapper::getViewByVob(Vob* vob)
{
	static VobView vobView;
	static OceanVobView oceanVobView;
	static RiggedVobView riggedVobView;

	if (dynamic_cast<nex::ProbeVob*>(vob)) {
		return mPbrProbeVobView.get();
	}
	else if (dynamic_cast<nex::OceanVob*>(vob)) {
		return &oceanVobView;
	}
	else if (dynamic_cast<nex::RiggedVob*>(vob)) {
		return &riggedVobView;
	}

	return &vobView;
}