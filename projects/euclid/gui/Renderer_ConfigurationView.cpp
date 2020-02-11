#include <gui/Renderer_ConfigurationView.hpp>
#include <nex/post_processing/AmbientOcclusion.hpp>
#include <nex/gui/ImGUI_Extension.hpp>

nex::gui::Renderer_ConfigurationView::Renderer_ConfigurationView(EuclidRenderer* renderer) : 
	mRenderer(renderer)
{
}

void nex::gui::Renderer_ConfigurationView::drawSelf()
{
	// render configuration properties
	ImGui::PushID(mId.c_str());

	AmbientOcclusionSelector* aoSelector = mRenderer->getAOSelector();
	bool useAmbientOcclusion = aoSelector->isAmbientOcclusionActive();

	if (ImGui::Checkbox("Ambient occlusion", &useAmbientOcclusion))
	{
		aoSelector->setUseAmbientOcclusion(useAmbientOcclusion);
	}

	if (useAmbientOcclusion)
	{
		std::stringstream ss;
		ss << AOTechnique::HBAO;
		std::string hbaoText = ss.str();

		const char* items[] = { hbaoText.c_str() };
		nex::AOTechnique selectedTechnique = aoSelector->getActiveAOTechnique();

		ImGui::SameLine(0, 70);
		if (ImGui::Combo("AO technique", (int*)&selectedTechnique, items, IM_ARRAYSIZE(items)))
		{
			std::cout << selectedTechnique << " is selected!" << std::endl;
			aoSelector->setAOTechniqueToUse(selectedTechnique);
		}
	}

	bool irradianceAA = mRenderer->getIrradianceAA();
	if (ImGui::Checkbox("antialias GI", &irradianceAA)) {
		mRenderer->setIrradianceAA(irradianceAA);
	}

	bool blurIrradiance = mRenderer->getBlurIrradiance();
	if (ImGui::Checkbox("blur GI", &blurIrradiance)) {
		mRenderer->setBlurIrradiance(blurIrradiance);
	}

	bool renderInHalfRes = mRenderer->getRenderGIinHalfRes();
	if (ImGui::Checkbox("render GI in half resolution", &renderInHalfRes)) {
		mRenderer->setRenderGIinHalfRes(renderInHalfRes);
		auto* out = mRenderer->getOutRT();
		mRenderer->updateRenderTargets(out->getWidth(), out->getHeight());

	}

	bool useDownSampledDepth = mRenderer->getDownSampledDepth();
	if (ImGui::Checkbox("Downsample Depth for GI", &useDownSampledDepth)) {
		mRenderer->setDownsampledDepth(useDownSampledDepth);
	}

	const auto& layerDescs = mRenderer->getRenderLayers();

	mRenderer->getActiveRenderLayer();

	size_t size = 1;
	for (const auto& layer : layerDescs) {
		size += layer.desc.size() + 1;
	}

	size_t cursor = 0;
	std::vector<char> flatDesc(size);
	for (const auto& layer : layerDescs) {
		memcpy(flatDesc.data() + cursor, layer.desc.data(), layer.desc.size());
		cursor += layer.desc.size();
		flatDesc[cursor] = '\0';
		++cursor;
	}

	flatDesc[cursor] = '\0'; // necessary to indicate that no more data follows!

	int index = static_cast<int>(mRenderer->getActiveRenderLayer());

	if (ImGui::Combo("Render layer", &index, flatDesc.data())) {
		mRenderer->setActiveRenderLayer(index);
	}

	/*nex::gui::Separator(2.0f);
	mTesselationConfig.drawGUI();
	*/
	nex::gui::Separator(2.0f);

	ImGui::Text("Probes:");
	
	{
		int index = static_cast<int>(mRenderer->getProbeSelectionAlg());
		if (ImGui::Combo("Probe selection", &index, "Nearest\0Four-Nearest interpolation")) {
			mRenderer->setProbeSelectionAlg((ProbeSelectionAlgorithm)index);
		}

	}
	

	nex::gui::Separator(2.0f);

	ImGui::PopID();
}