#include <nex/gui/WaterGenerator.hpp>

nex::gui::WaterGenerator::WaterGenerator(nex::Scene* scene, CascadedShadow* csm, PSSR* pssr) : Drawable(), 
mScene(scene), mCsm(csm), mPssr(pssr)
{
}

nex::gui::WaterGenerator::~WaterGenerator() = default;

void nex::gui::WaterGenerator::setScene(nex::Scene* scene)
{
	mScene = scene;
}

void nex::gui::WaterGenerator::drawSelf()
{
	ImGui::Text("Ocean Generation:");

	/*
	unsigned N, 
			unsigned maxWaveLength, 
			float dimension,
			float spectrumScale, 
			const glm::vec2& windDirection, 
			float windSpeed, 
			float periodTime,
			const glm::uvec2& tileCount,
	*/

	ImGui::Button("Generate");
}