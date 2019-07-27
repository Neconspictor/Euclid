#include <gui/ProbeGeneratorView.hpp>
#include <nex/gui/Util.hpp>
nex::gui::ProbeGeneratorView::ProbeGeneratorView(std::string title, nex::gui::MainMenuBar* menuBar, nex::gui::Menu* menu, nex::Scene* scene) : 
	MenuWindow(std::move(title), menuBar, menu),
	mScene(scene)
{
}

nex::gui::ProbeGeneratorView::~ProbeGeneratorView() = default;

void nex::gui::ProbeGeneratorView::setScene(nex::Scene* scene)
{
	mScene = scene;
}

void nex::gui::ProbeGeneratorView::drawSelf()
{
	ImGui::DragFloat3("Probe position", (float*)& mPosition, 0.1f, 0.0f, 0.0f, "%.5f");
}