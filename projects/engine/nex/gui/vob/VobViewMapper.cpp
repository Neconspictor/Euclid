#include <nex/gui/vob/VobViewMapper.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/GI/Probe.hpp>
#include <nex/water/Ocean.hpp>
#include <nex/gui/vob/OceanVobView.hpp>
#include <nex/gui/vob/ProbeVobView.hpp>
#include <nex/gui/vob/RiggedVobView.hpp>


std::vector<std::unique_ptr<nex::gui::VobView>> nex::gui::VobViewMapper::mViews;

void nex::gui::VobViewMapper::init(ProbeManager* probeManager, nex::Window* window)
{
	mViews.push_back(std::make_unique<VobView>(window));
	mViews.push_back(std::make_unique<ProbeVobView>(probeManager, window));
	mViews.push_back(std::make_unique<OceanVobView>(window));
	mViews.push_back(std::make_unique<RiggedVobView>(window));	
}

nex::gui::VobView* nex::gui::VobViewMapper::getViewByVob(Vob* vob)
{
	if (dynamic_cast<nex::ProbeVob*>(vob)) {
		return mViews[1].get();
	}
	else if (dynamic_cast<nex::OceanVob*>(vob)) {
		return mViews[2].get();
	}
	else if (dynamic_cast<nex::RiggedVob*>(vob)) {
		return mViews[3].get();
	}

	return mViews[0].get();
}