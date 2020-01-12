#include <nex/gui/ProbeGeneratorView.hpp>
#include <nex/gui/ImGUI_Extension.hpp>
#include <nex/pbr/ProbeGenerator.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/pbr/PbrProbe.hpp>
#include <nex/resource/ResourceLoader.hpp>

nex::gui::ProbeGeneratorView::ProbeGeneratorView(std::string title, 
	nex::gui::MainMenuBar* menuBar, 
	nex::gui::Menu* menu, nex::ProbeGenerator* generator,
	nex::Camera* camera) :
	MenuWindow(std::move(title), menuBar, menu),
	mGenerator(generator),
	mCamera(camera),
	mPlacementOffset(2.0f)
{
}

nex::gui::ProbeGeneratorView::~ProbeGeneratorView() = default;

void nex::gui::ProbeGeneratorView::setVisible(bool visible, bool recursive)
{
	bool oldVisibleState = isVisible();
	MenuWindow::setVisible(visible, recursive);
	mGenerator->show(visible);

	if (visible && !oldVisibleState) {
		mGenerator->update(mCamera->getPosition() + mPlacementOffset * mCamera->getLook(), 
			mGenerator->getInfluenceRadius());
	}
}

void nex::gui::ProbeGeneratorView::drawSelf()
{
	auto position = mGenerator->getProbePosition();
	auto influenceRadius = mGenerator->getInfluenceRadius();
	ImGui::DragFloat3("Position", (float*)& position, 0.1f, 0.0f, 0.0f, "%.5f");
	ImGui::DragFloat("Influence radius", &influenceRadius, 0.1f, 0.0f, 0.0f, "%.5f");
	
	ImGui::Dummy(ImVec2(0, 10));
	nex::gui::Separator(2.0f);
	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text("Position placement related configuration:");
	ImGui::DragFloat("Camera look offset", (float*)& mPlacementOffset, 0.1f, 0.0f, 0.0f, "%.5f");

	if (ImGui::Button("Position in front at camera")) {
		position = mCamera->getPosition() + mPlacementOffset * mCamera->getLook();
	}

	mGenerator->update(position, influenceRadius);

	ImGui::Dummy(ImVec2(0, 10));
	nex::gui::Separator(2.0f);
	ImGui::Dummy(ImVec2(0, 40));

	if (ImGui::Button("Generate")) {


		ResourceLoader::get()->enqueue([=]()->nex::Resource * {
			RenderEngine::getCommandQueue()->push([=]() {
					mGenerator->generate();
					setVisible(false);
				});

			return nullptr;
			});
	}
}